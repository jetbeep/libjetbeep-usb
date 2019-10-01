#ifndef JETBEEP_DEVICE_DETECTION_H
#define JETBEEP_DEVICE_DETECTION_H

#include "../utils/platform.h"

#include <thread>
#include <stdint.h>
#include <stddef.h>

#ifdef PLATFORM_OSX
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

namespace jetbeep {
	typedef struct VidPid {
		uint16_t vid;
		uint16_t pid;
	} VidPid;


	class DeviceDetection {
	public:
		DeviceDetection();
		virtual ~DeviceDetection();

		void setup() noexcept(false);
	static size_t vidPidCount;
	static VidPid validVidPids[];
	static bool isValidVidPid(const VidPid &vidpid);
	private:
#ifdef PLATFORM_OSX
		CFRunLoopRef _loop;
		io_iterator_t _iterator;
		std::thread _thread;
		IONotificationPortRef	_notify_port;

		VidPid getVidPid(const io_service_t &service);

		static void DeviceAdded(void *refCon, io_iterator_t iterator);

		void runLoop();
#endif
	};
}

#endif
