#include "detection.h"
#include <stdexcept>
#include <iostream>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>

#include <boost/asio.hpp>

using namespace jetbeep;
using namespace std;
using namespace boost::asio;

size_t DeviceDetection::vidPidCount = 2;
VidPid DeviceDetection::validVidPids[]= { {0x04d8, 0x00df}, {0x1915, 0x776A} };

bool DeviceDetection::isValidVidPid(const VidPid &vidpid) {
	for (int i = 0; i < vidPidCount; ++ i) {
		if (validVidPids[i].vid == vidpid.vid && validVidPids[i].pid == vidpid.pid) {
			return true;
		}
	}
	return false;
}

DeviceDetection::DeviceDetection()
:_loop(NULL), _iterator(0), _notify_port(NULL) {

}

DeviceDetection::~DeviceDetection() {
	if (_loop != NULL) {
		CFRunLoopStop(_loop);
	}

	if (_thread.joinable()) {
		_thread.join();
	}
}

void DeviceDetection::DeviceAdded(void *refCon, io_iterator_t iterator) {
	io_service_t service;
	DeviceDetection* detection = reinterpret_cast<DeviceDetection*>(refCon);

	while ((service = IOIteratorNext(iterator)))
	{
	    cout << "Hey, I found a service!" << endl;

	    auto vidPid = detection->getVidPid(service);

	    if (DeviceDetection::isValidVidPid(vidPid)) {
	    	cout << "found jetbeep device, vid: " << vidPid.vid << " pid: " << vidPid.pid<< endl;
	    }

	    IOObjectRelease(service); // yes, you have to release this
	}
}

void DeviceDetection::setup() noexcept(false) {
	kern_return_t kr;
	CFMutableDictionaryRef 	matchingDict;
	CFRunLoopRef				gRunLoop;

	matchingDict = IOServiceMatching(kIOSerialBSDServiceValue/*kIOUSBDeviceClassName*/);

	if (matchingDict == NULL) {
		throw runtime_error("unable to find USB class name");
	}

	_notify_port = IONotificationPortCreate(kIOMasterPortDefault);

	kr = IOServiceAddMatchingNotification(
				_notify_port, // notifyPort
				kIOFirstMatchNotification, // notificationType
				matchingDict, // matching
				DeviceAdded, // callback
				this, // refCon
				&_iterator // notification
			);
	if (kr != KERN_SUCCESS) {
		throw runtime_error("unable to create matching notification");
	}

	_thread = thread(&DeviceDetection::runLoop, this);
}

void DeviceDetection::runLoop() {
	DeviceAdded(NULL, _iterator);

	CFRunLoopSourceRef runLoopSource = NULL;

	runLoopSource = IONotificationPortGetRunLoopSource(_notify_port);

	_loop = CFRunLoopGetCurrent();
	CFRunLoopAddSource(_loop , runLoopSource, kCFRunLoopDefaultMode);

	CFRunLoopRun();
	cout<<"aw";
	cout.flush();
}

VidPid DeviceDetection::getVidPid(const io_service_t &service) {
	int vid = 0, pid = 0;
	CFTypeRef cf_vendor, cf_product;
	VidPid return_value = { 0, 0 };

	// Search properties among parents of the current modemService
	cf_vendor = (CFTypeRef) IORegistryEntrySearchCFProperty(service, kIOServicePlane,
	                                            CFSTR("idVendor"),
	                                            kCFAllocatorDefault,
	                                            kIORegistryIterateRecursively
	                                            | kIORegistryIterateParents);

	cf_product = (CFTypeRef) IORegistryEntrySearchCFProperty(service, kIOServicePlane,
	                                             CFSTR("idProduct"),
	                                             kCFAllocatorDefault,
	                                             kIORegistryIterateRecursively
	                                             | kIORegistryIterateParents);

	// Decode & print VID & PID
	if (cf_vendor && cf_product &&
	    CFNumberGetValue((CFNumberRef)cf_vendor , kCFNumberIntType, &vid) &&
	    CFNumberGetValue((CFNumberRef)cf_product, kCFNumberIntType, &pid)) {
	}

	if (cf_vendor)  CFRelease(cf_vendor);
	if (cf_product) CFRelease(cf_product);

	return_value.vid = vid;
	return_value.pid = pid;

	return return_value;
}
