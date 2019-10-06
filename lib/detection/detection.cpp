#include <stdexcept>
#include <iostream>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>

#include <boost/asio.hpp>
#include <detection/detection.hpp>

using namespace JetBeep;
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

DeviceDetection::DeviceDetection(DeviceCallback callback)
:m_loop(NULL), m_iterator(0), m_notify_port(NULL), m_log("detection"), callback(callback) {

}

DeviceDetection::~DeviceDetection() {
	if (m_loop != NULL) {
		CFRunLoopStop(m_loop);
	}

	if (m_thread.joinable()) {
		m_thread.join();
	}
}

void DeviceDetection::DeviceAdded(void *refCon, io_iterator_t iterator) {
	io_service_t service;
	DeviceDetection* detection = reinterpret_cast<DeviceDetection*>(refCon);

	while ((service = IOIteratorNext(iterator)))
	{
	    auto vidPid = detection->getVidPid(service);

	    if (DeviceDetection::isValidVidPid(vidPid)) {
	    	Device device = { vidPid.vid, vidPid.pid, detection->getDevicePath(service) };
	    	auto callback = detection->callback;

	    	if (callback != nullptr) {
	    		callback(ADDED, device);
	    	}
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

	m_notify_port = IONotificationPortCreate(kIOMasterPortDefault);

	kr = IOServiceAddMatchingNotification(
				m_notify_port, // notifyPort
				kIOFirstMatchNotification, // notificationType
				matchingDict, // matching
				DeviceAdded, // callback
				this, // refCon
				&m_iterator // notification
			);
	if (kr != KERN_SUCCESS) {
		throw runtime_error("unable to create matching notification");
	}

	m_thread = thread(&DeviceDetection::runLoop, this);
}

void DeviceDetection::runLoop() {
	DeviceAdded(this, m_iterator);

	CFRunLoopSourceRef runLoopSource = NULL;

	runLoopSource = IONotificationPortGetRunLoopSource(m_notify_port);

	m_loop = CFRunLoopGetCurrent();
	CFRunLoopAddSource(m_loop , runLoopSource, kCFRunLoopDefaultMode);

	CFRunLoopRun();
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

string DeviceDetection::getDevicePath(const io_service_t &service) {
	string result;

	CFStringRef device_path = (CFStringRef) IORegistryEntrySearchCFProperty (service,
	  kIOServicePlane,
	  CFSTR (kIOCalloutDeviceKey),
	  kCFAllocatorDefault,
	  kIORegistryIterateRecursively );


	if (!device_path) {
		return string();
	}

	const char *ptr = CFStringGetCStringPtr(device_path, kCFStringEncodingUTF8);
	size_t len = CFStringGetLength(device_path);

	if (ptr != NULL && len != 0) {
		result.assign(ptr, len);
	}

	if (device_path) {
		CFRelease(device_path);
	}

	return result;
}
