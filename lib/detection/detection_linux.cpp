#include "../utils/platform.hpp"

#ifdef PLATFORM_LINUX
#include "detection.hpp"
#include "../utils/logger.hpp"

#include <thread>
#include <unordered_map>
#include <libudev.h>

using namespace JetBeep;
using namespace std;

class DeviceDetection::Impl {
  public:
		Impl(DeviceDetectionCallback *callback);

		void startMonitoring();
		void stopMonitoring();
		void detectConnected();

		virtual ~Impl();
	private:
		DeviceDetectionCallback *m_callback;
		Logger m_log;
		struct udev * udev;
		
		

		//VidPid getVidPid(const io_service_t &service);
		//std::string getDevicePath(const io_service_t &service);

};

// DeviceDetection implementation

DeviceDetection::Impl::Impl(DeviceDetectionCallback *callback) 
	:m_callback(callback), m_log("detection") {
		udev = udev_new();
		if (udev == NULL) {
			throw "Unable to initialize UDEV";
		}
}

DeviceDetection::Impl::~Impl() {
	stopMonitoring();
	udev_unref(udev);
}

void DeviceDetection::Impl::stopMonitoring() {
	
}

void DeviceDetection::Impl::startMonitoring() {
	
}

void DeviceDetection::Impl::detectConnected() {
	struct udev_device *dev, *devParent;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *deviceEntry, *attrsEntry;

	enumerate = udev_enumerate_new(udev);
	if (!enumerate) {
		throw "UDEV: Unable to initialize UDEV enumerate";
	}

	udev_enumerate_add_match_subsystem(enumerate, "tty");
	udev_enumerate_scan_devices(enumerate);

	devices = udev_enumerate_get_list_entry(enumerate);
	if (!devices) {
		throw "UDEV: Unable to get devices list";
	}

	udev_list_entry_foreach(deviceEntry, devices) {
		const char *path, *tmp, *vendorId, *productId;

		path = udev_list_entry_get_name(deviceEntry);
		dev = udev_device_new_from_syspath(udev, path);
		if (!dev) continue;

		struct udev_list_entry * attrsAvailable = udev_device_get_sysattr_list_entry(dev);

		const char * devNode = udev_device_get_devnode(dev);
		
		if (!devNode) {
			udev_device_unref(dev);
			continue;
		}

		devParent = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
		if (devParent) {
			vendorId = udev_device_get_sysattr_value(devParent,"idVendor");
			productId = udev_device_get_sysattr_value(devParent,"idProduct");

			VidPid deviceIds;
			deviceIds.vid = strtol(vendorId, NULL, 16);
			deviceIds.pid = strtol(productId, NULL, 16);
			
			if (DeviceDetection::isValidVidPid(deviceIds)) {
				m_log.d() << "found JetBeep device: " << vendorId << ":" << productId << " path: "<< devNode << Logger::endl;
			}
			udev_device_unref(devParent);
		}
		
		udev_device_unref(dev);
	}
	udev_enumerate_unref(enumerate);
	udev_unref(udev);
}


// DeviceDetection

DeviceDetection::DeviceDetection(DeviceDetectionCallback callback)
: callback(callback), m_impl(new Impl(&this->callback)) {}

DeviceDetection::~DeviceDetection() {}

void DeviceDetection::start() { 
	m_impl->detectConnected();
	m_impl->startMonitoring();
}
void DeviceDetection::stop() { m_impl->stopMonitoring(); }

#endif