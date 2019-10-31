#include "../utils/platform.hpp"

#ifdef PLATFORM_LINUX
#include "detection.hpp"
#include "../utils/logger.hpp"

#include <thread>
#include <atomic>
#include <libudev.h>
#include <unistd.h>
#include <stdexcept>

using namespace JetBeep;
using namespace std;

typedef struct udev_device uDev;

class DeviceDetection::Impl {
  public:
	Impl(DeviceDetectionCallback *callback);
	void startMonitoring();
	void stopMonitoring();
	void detectConnected();
	virtual ~Impl();

  private:
	DeviceDetectionCallback *m_callback = nullptr;
	Logger m_log;
	struct udev *udev = nullptr;
	std::atomic<bool> isMonActive;
	std::thread m_thread;
	bool checkTTYParent(uDev *, DeviceCandidate *);
	bool checkDevVIDPID(uDev *, DeviceCandidate *);
	void udevMonitorLoop();
};

// DeviceDetection implementation

DeviceDetection::Impl::Impl(DeviceDetectionCallback *callback)
	: m_callback(callback), m_log("detection") {
	udev = udev_new();
	if (udev == nullptr) {
		throw runtime_error("Unable to initialize UDEV");
	}
	isMonActive.store(false);
}

DeviceDetection::Impl::~Impl() {
	stopMonitoring();
	udev_unref(udev);
}

bool DeviceDetection::Impl::checkDevVIDPID(uDev *dev, DeviceCandidate *deviceCandidate) {
	if (!dev) {
		return false;
	}
		
	auto vendorId = udev_device_get_sysattr_value(dev, "idVendor");
	auto productId = udev_device_get_sysattr_value(dev, "idProduct");
	const char * propModelId = nullptr;
	const char * propVendorId = nullptr;

	if (!vendorId || !productId) {
		propModelId = udev_device_get_property_value(dev, "ID_MODEL_ID");
		propVendorId = udev_device_get_property_value(dev, "ID_VENDOR_ID");
		if (!propModelId || !propVendorId) return false;
	}

	VidPid deviceIds;
	deviceIds.vid = strtol(vendorId ? vendorId : propVendorId, nullptr, 16);
	deviceIds.pid = strtol(productId ? productId : propModelId, nullptr, 16);

	if (DeviceDetection::isValidVidPid(deviceIds)) {
		deviceCandidate->vid = deviceIds.vid;
		deviceCandidate->pid = deviceIds.pid;
		return true;
	} 
	return false;
}

bool DeviceDetection::Impl::checkTTYParent(uDev *ttyDevice, DeviceCandidate *deviceCandidate) {
	uDev *devParent = udev_device_get_parent_with_subsystem_devtype(ttyDevice, "usb", "usb_device");

	bool result = checkDevVIDPID(devParent, deviceCandidate);

	udev_device_unref(devParent);

	return result;
}

void DeviceDetection::Impl::stopMonitoring() {
	isMonActive.store(false);

	if (m_thread.joinable()) {
		m_thread.join();
	}

	m_log.d() << "Udev mon stopped" << Logger::endl;
}

void DeviceDetection::Impl::startMonitoring() {
	if (isMonActive.load())
		return;

	isMonActive.store(true);

	m_thread = thread(&DeviceDetection::Impl::udevMonitorLoop, this);
}

void DeviceDetection::Impl::udevMonitorLoop() {
	auto callback = *this->m_callback;

	struct udev_enumerate *enumerate = nullptr;
	uDev *dev = nullptr;
	struct udev_monitor *mon = nullptr;
	int fd;

	if (!udev || callback == nullptr) {
		return;
	}

	mon = udev_monitor_new_from_netlink(udev, "udev");
	udev_monitor_filter_add_match_subsystem_devtype(mon, "tty", nullptr);
	udev_monitor_enable_receiving(mon);
	fd = udev_monitor_get_fd(mon);

	m_log.d() << "Udev mon started" << Logger::endl;

	while (isMonActive.load()) {
		fd_set fds;
		struct timeval tv;
		int ret;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec = 0;
		tv.tv_usec = 250 * 1000; //ms timeout for fd changes

		ret = select(fd + 1, &fds, nullptr, nullptr, &tv);

		if (ret > 0 && FD_ISSET(fd, &fds)) {
			dev = udev_monitor_receive_device(mon);
			if (!dev) {
				continue;
			}

			string action(udev_device_get_action(dev));

			m_log.d() << "Udev mon event: " << action << Logger::endl;

			DeviceCandidate candidate;
			auto devNode = udev_device_get_devnode(dev);
			if (!devNode) {
				continue;
			}

			candidate.path = string(devNode);

			if (action == "add") {
				if (checkTTYParent(dev, &candidate)) {
					callback(DeviceDetectionEvent::added, candidate);
				}
			} else if (action == "remove") {
				if (checkDevVIDPID(dev, &candidate)) {
					callback(DeviceDetectionEvent::removed, candidate);
				}
			}
			//udev_device_unref(dev); //This causes segmentation fault... in some cases
		}
	}

	udev_monitor_unref(mon);
}

void DeviceDetection::Impl::detectConnected() {
	uDev *dev = nullptr;
	struct udev_enumerate *enumerate = nullptr;
	struct udev_list_entry *devices = nullptr, *deviceEntry = nullptr;

	enumerate = udev_enumerate_new(udev);
	if (!enumerate) {
		throw runtime_error("UDEV: Unable to initialize UDEV enumerate");
	}

	udev_enumerate_add_match_subsystem(enumerate, "tty");
	udev_enumerate_scan_devices(enumerate);

	devices = udev_enumerate_get_list_entry(enumerate);
	if (!devices) {
		throw runtime_error("UDEV: Unable to get devices list");
	}

	udev_list_entry_foreach(deviceEntry, devices) {
		const char *path, *tmp, *vendorId, *productId;

		path = udev_list_entry_get_name(deviceEntry);
		dev = udev_device_new_from_syspath(udev, path);
		if (!dev) {
			continue;
		}

		struct udev_list_entry *attrsAvailable = udev_device_get_sysattr_list_entry(dev);

		const char *devNode = udev_device_get_devnode(dev);

		if (!devNode) {
			udev_device_unref(dev);
			continue;
		}

		DeviceCandidate deviceCandidate = {0, 0, string(devNode)};

		if (checkTTYParent(dev, &deviceCandidate)) {
			auto callback = *this->m_callback;

			if (callback != nullptr) {
				callback(DeviceDetectionEvent::added, deviceCandidate);
			}
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

void DeviceDetection::stop() { 
	m_impl->stopMonitoring(); 
}

#endif