#ifndef JETBEEP_DEVICE_DETECTION_H
#define JETBEEP_DEVICE_DETECTION_H

#include <memory>
#include <string>
#include <stdint.h>
#include <stddef.h>

namespace JetBeep {
	typedef struct VidPid {
		uint16_t vid;
		uint16_t pid;
	} VidPid;

	typedef enum class DeviceDetectionEvent {
		added,
		removed
	} DeviceDetectionEvent;

	typedef struct DeviceCandidate {
		uint16_t vid;
		uint16_t pid;
		std::string path;
	} DeviceCandidate;

	typedef void (*DeviceDetectionCallback)(const DeviceDetectionEvent&, const DeviceCandidate&);

	class DeviceDetection {
	public:
		DeviceDetection(DeviceDetectionCallback callback = nullptr);
		virtual ~DeviceDetection();

		void start() noexcept(false);
		void stop() noexcept(false);
		DeviceDetectionCallback callback;

		static size_t vidPidCount;
		static VidPid validVidPids[];
		static bool isValidVidPid(const VidPid &vidpid);
	private:
		class Impl;
		std::unique_ptr<Impl> m_impl;
	};
}

#endif
