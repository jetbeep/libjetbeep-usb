#include <stdexcept>
#include <iostream>

#include "detection.hpp"

using namespace JetBeep;
using namespace std;

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
