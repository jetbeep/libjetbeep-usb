#include "../utils/platform.hpp"

#ifdef PLATFORM_LINUX
#include "detection.hpp"
#include "../utils/logger.hpp"

#include <thread>
#include <unordered_map>
#include <libudev.h>
#include <string>

using namespace JetBeep;
using namespace std;

typedef struct udev_device uDev;

class DeviceDetection::Impl
{
public:
	Impl(DeviceDetectionCallback *callback);

	void startMonitoring();
	void stopMonitoring();
	void detectConnected();

	virtual ~Impl();

private:
	DeviceDetectionCallback *m_callback;
	Logger m_log;
	struct udev *udev;
	bool isMonActive = false;

	class UdevUSBDevice {
		public:
		string type;
		string path;
		string node;
		string subsystem;
	};

	bool checkTTYParent(uDev *ttyDevice, DeviceCandidate &candidate);
	bool checkDevVIDPID(uDev *ttyDevice, DeviceCandidate &candidate);
};

// DeviceDetection implementation

DeviceDetection::Impl::Impl(DeviceDetectionCallback *callback)
	: m_callback(callback), m_log("detection")
{
	udev = udev_new();
	if (udev == NULL)
	{
		throw "Unable to initialize UDEV";
	}
}

DeviceDetection::Impl::~Impl()
{
	stopMonitoring();
	udev_unref(udev);
}

bool DeviceDetection::Impl::checkDevVIDPID(uDev *dev, DeviceCandidate &deviceCandidate)
{
	if (!dev) {
		return false;
	}
		
	auto vendorId = udev_device_get_sysattr_value(dev, "idVendor");
	auto productId = udev_device_get_sysattr_value(dev, "idProduct");

	m_log.d() << "vid:pid "<<vendorId<< ":" <<productId << Logger::endl;


	if (!vendorId || !productId) {
		return false;
	}

	VidPid deviceIds;
	deviceIds.vid = strtol(vendorId, NULL, 16);
	deviceIds.pid = strtol(productId, NULL, 16);

	if (DeviceDetection::isValidVidPid(deviceIds)) {
		deviceCandidate.vid = deviceIds.vid;
		deviceCandidate.pid = deviceIds.pid;
		return true;
	} 
	return false;
}

bool DeviceDetection::Impl::checkTTYParent(uDev *ttyDevice, DeviceCandidate &deviceCandidate)
{
	uDev *devParent = udev_device_get_parent_with_subsystem_devtype(ttyDevice, "usb", "usb_device");

	bool result = checkDevVIDPID(devParent, deviceCandidate);

	udev_device_unref(devParent);

	return result;
}

void DeviceDetection::Impl::stopMonitoring()
{
	isMonActive = false;
}

void DeviceDetection::Impl::startMonitoring()
{
	if (isMonActive)
		return;
	isMonActive = true;
	auto callback = *this->m_callback;

	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	uDev *dev;
	struct udev_monitor *mon;
	int fd;

	if (!udev || callback == nullptr) {
		return;
	}

	mon = udev_monitor_new_from_netlink(udev, "udev");
	udev_monitor_filter_add_match_subsystem_devtype(mon, "tty", NULL);
	udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
	udev_monitor_enable_receiving(mon);
	fd = udev_monitor_get_fd(mon);

	m_log.d() << "Udev mon started" << Logger::endl;

	UdevUSBDevice lastRemovedTTYdev;

	while (isMonActive)
	{
		fd_set fds;
		struct timeval tv;
		int ret;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		ret = select(fd + 1, &fds, NULL, NULL, &tv);

		if (ret > 0 && FD_ISSET(fd, &fds))
		{
			dev = udev_monitor_receive_device(mon);
			if (!dev) continue;

			string action(udev_device_get_action(dev));

			m_log.d() << "Udev mon event: " << action << Logger::endl;

			auto subsystem = udev_device_get_subsystem(dev);
			auto devtype = udev_device_get_devtype(dev);
			auto devpath = udev_device_get_devpath(dev);

			if (!subsystem) {
				continue;
			}

			DeviceCandidate candidate;

			if (action == "add") {
				auto devNode = udev_device_get_devnode(dev);
				if (devNode && string(subsystem) == "tty" && checkTTYParent(dev, candidate)) {
					callback(ADDED, candidate);
				}
			} else if (action == "remove") {
				if (string(subsystem) == "usb" && checkDevVIDPID(dev, candidate)) {
					//removed last
					string usbDevPath(devpath ? devpath : "");
					if (lastRemovedTTYdev.path.empty() 
						// make sure last removed tty device is usb device child
						// /devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2.2 + /1-2.2:1.0/tty/ttyACM0
						|| !(lastRemovedTTYdev.path.compare(0, usbDevPath.length(), usbDevPath) == 0)) {
						m_log.d() << "JetBeep device removed - unknown tty path" << Logger::endl;
					}

					candidate.path = lastRemovedTTYdev.node;
					lastRemovedTTYdev.path = "";
					lastRemovedTTYdev.node = "";
					callback(REMOVED, candidate);
				} else if (string(subsystem) == "tty") {
					//removed first
					auto devNode = udev_device_get_devnode(dev);
					lastRemovedTTYdev.node = devNode ? string(devNode) : "";
					lastRemovedTTYdev.path = devpath ? string(devpath) : "";
					m_log.d() << "tty " <<  devNode << Logger::endl;

				}
			}

			udev_device_unref(dev);
		}
	}

	udev_monitor_unref(mon);
}

void DeviceDetection::Impl::detectConnected()
{
	uDev *dev, *devParent;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *deviceEntry, *attrsEntry;

	enumerate = udev_enumerate_new(udev);
	if (!enumerate)
	{
		throw "UDEV: Unable to initialize UDEV enumerate";
	}

	udev_enumerate_add_match_subsystem(enumerate, "tty");
	udev_enumerate_scan_devices(enumerate);

	devices = udev_enumerate_get_list_entry(enumerate);
	if (!devices)
	{
		throw "UDEV: Unable to get devices list";
	}

	udev_list_entry_foreach(deviceEntry, devices)
	{
		const char *path, *tmp, *vendorId, *productId;

		path = udev_list_entry_get_name(deviceEntry);
		dev = udev_device_new_from_syspath(udev, path);
		if (!dev)
			continue;

		struct udev_list_entry *attrsAvailable = udev_device_get_sysattr_list_entry(dev);

		const char *devNode = udev_device_get_devnode(dev);

		if (!devNode)
		{
			udev_device_unref(dev);
			continue;
		}

		DeviceCandidate deviceCandidate = {0, 0, string(devNode)};

		if (checkTTYParent(dev, deviceCandidate))
		{
			auto callback = *this->m_callback;

			if (callback != nullptr)
			{
				callback(ADDED, deviceCandidate);
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

void DeviceDetection::start()
{
	m_impl->detectConnected();
	m_impl->startMonitoring();
}
void DeviceDetection::stop() { m_impl->stopMonitoring(); }

#endif