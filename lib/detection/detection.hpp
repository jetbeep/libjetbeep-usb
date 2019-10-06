#ifndef JETBEEP_DEVICE_DETECTION_H
#define JETBEEP_DEVICE_DETECTION_H

#include <thread>
#include <stdint.h>
#include <stddef.h>
#include <unordered_map>
#include "../utils/logger.hpp"
#include "../utils/platform.hpp"

#ifdef PLATFORM_OSX
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

namespace JetBeep {
	typedef struct VidPid {
		uint16_t vid;
		uint16_t pid;
	} VidPid;

	typedef enum DeviceEvent {
		ADDED,
		REMOVED
	} DeviceEvent;

	typedef struct Device {
		uint16_t vid;
		uint16_t pid;
		std::string path;
	} Device;

	typedef void (*DeviceCallback)(const DeviceEvent&, const Device&);

	class DeviceDetection {
	public:
		DeviceDetection(DeviceCallback callback = nullptr);
		virtual ~DeviceDetection();

		void setup() noexcept(false);
		DeviceCallback callback;

		static size_t vidPidCount;
		static VidPid validVidPids[];
		static bool isValidVidPid(const VidPid &vidpid);
	private:
		Logger m_log;
#ifdef PLATFORM_OSX
		CFRunLoopRef m_loop;
		io_iterator_t m_iterator;
		std::thread m_thread;
		IONotificationPortRef m_notifyPort;
		std::unordered_map<std::string, std::pair<Device, io_service_t>> m_trackedDevices;

		VidPid getVidPid(const io_service_t &service);
		std::string getDevicePath(const io_service_t &service);

		static void deviceAdded(void *refCon, io_iterator_t iterator);
		static void deviceRemoved(void *refCon, io_service_t service, natural_t messageType, void *messageArgument);

		void runLoop();
#endif
	};
}

#endif
