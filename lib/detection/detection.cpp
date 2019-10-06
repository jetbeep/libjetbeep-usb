#include <stdexcept>
#include <iostream>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>

#include "../boost_1_71_0/boost/asio.hpp"
#include "detection.hpp"

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

	for (auto it = m_tracked_devices.begin(); it != m_tracked_devices.end(); ++it) {
		auto service = it->second.second;

		IOObjectRelease(service);
	}
}

void DeviceDetection::deviceAdded(void *refCon, io_iterator_t iterator) {
	io_service_t service;
	kern_return_t kr;
	DeviceDetection* detection = reinterpret_cast<DeviceDetection*>(refCon);

	while ((service = IOIteratorNext(iterator)))
	{
	    auto vidPid = detection->getVidPid(service);

	    if (DeviceDetection::isValidVidPid(vidPid)) {
	    	Device device = { vidPid.vid, vidPid.pid, detection->getDevicePath(service) };
	    	io_service_t remove_service;

	    	kr = IOServiceAddInterestNotification(
	    			detection->m_notify_port, // notifyPort
					service, // service
	    			kIOGeneralInterest, // interestType
	    			deviceRemoved, // callback
	    			detection, // refCon
	    			&remove_service // notification
	    			);

	    	if (KERN_SUCCESS != kr) {
	    		detection->m_log.e() << "unable to add device remove callback: "<< kr << Logger::endl;
	    		IOObjectRelease(service);
	    		continue;
	    	}

	    	detection->m_tracked_devices[device.path] = make_pair(device, remove_service);

	    	auto callback = detection->callback;

	    	if (callback != nullptr) {
	    		callback(ADDED, device);
	    	}
	    }

	    IOObjectRelease(service); // yes, you have to release this
	}
}

void DeviceDetection::deviceRemoved(void *refCon, io_service_t service, natural_t messageType, void *messageArgument) {
	kern_return_t kr;
	DeviceDetection* detection = reinterpret_cast<DeviceDetection*>(refCon);

	if (messageType != kIOMessageServiceIsTerminated) {
		return;
	}

	auto path = detection->getDevicePath(service);
	auto devicePair = detection->m_tracked_devices[path];
	auto device = devicePair.first;
	auto notification = devicePair.second;

	detection->m_tracked_devices.erase(path);

	kr = IOObjectRelease(notification);

	if (KERN_SUCCESS != kr) {
		detection->m_log.e() << "unable to relase service in deviceRemoved: " << kr << Logger::endl;
		return;
	}

	auto callback = detection->callback;

	if (callback != nullptr) {
		callback(REMOVED, device);
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
				deviceAdded, // callback
				this, // refCon
				&m_iterator // notification
			);
	if (kr != KERN_SUCCESS) {
		throw runtime_error("unable to create matching notification");
	}

	m_thread = thread(&DeviceDetection::runLoop, this);
}

void DeviceDetection::runLoop() {
	deviceAdded(this, m_iterator);

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
