#include "../utils/platform.hpp"

#ifdef PLATFORM_LINUX
#include "detection.hpp"
#include "../utils/logger.hpp"

#include <thread>
#include <unordered_map>
#include <libudev.h>
#include <unistd.h>

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
	const char * propModelId = NULL;
	const char * propVendorId = NULL;

	if (!vendorId || !productId) {
		propModelId = udev_device_get_property_value(dev, "ID_MODEL_ID");
		propVendorId = udev_device_get_property_value(dev, "ID_VENDOR_ID");
		if (!propModelId || !propVendorId) return false;
	}

	VidPid deviceIds;
	deviceIds.vid = strtol(vendorId ? vendorId : propVendorId, NULL, 16);
	deviceIds.pid = strtol(productId ? productId : propModelId, NULL, 16);

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
	udev_monitor_enable_receiving(mon);
	fd = udev_monitor_get_fd(mon);

	m_log.d() << "Udev mon started" << Logger::endl;

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

			DeviceCandidate candidate;
			auto devNode = udev_device_get_devnode(dev);
			if (!devNode) continue;

			candidate.path = string(devNode);

			if (action == "add") {
				if (checkTTYParent(dev, candidate)) {
					callback(ADDED, candidate);
				}
			} else if (action == "remove") {
				if (checkDevVIDPID(dev, candidate)) {
					callback(REMOVED, candidate);
				}
			}

			udev_device_unref(dev);
		}
		usleep(250*1000);
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