#include <iostream>
#include <stdexcept>

#include "detection.hpp"

using namespace JetBeep;
using namespace std;

// TODO add Nordic Dev kits
size_t DeviceDetection::vidPidCount = 3;
VidPid DeviceDetection::validVidPids[] = {{0x04d8, 0x00df}, {0x1915, 0x776A}, {0x1915, 0x521F /*legacy bootloader PID 52840*/}};

bool DeviceDetection::isValidVidPid(const VidPid& vidpid) {
  for (int i = 0; i < vidPidCount; ++i) {
    if (validVidPids[i].vid == vidpid.vid && validVidPids[i].pid == vidpid.pid) {
      return true;
    }
  }
  return false;
}

bool DeviceCandidate::operator==(const DeviceCandidate& other) {
  return path == other.path;
}

bool DeviceCandidate::operator!=(const DeviceCandidate& other) {
  return !(*this == other);
}