#ifndef DFU_MAIN__HPP
#define DFU_MAIN__HPP

#include "../lib/libjetbeep.hpp"

using namespace std;

enum class DeviceBootState { APP, BOOTLOADER, UNKNOWN };
enum class DeviceConfigState { CONFIGURED, PARTIAL, BLANK, UNKNOWN };

struct DeviceInfo {
  DeviceBootState bootState = DeviceBootState::UNKNOWN;
  DeviceConfigState configState = DeviceConfigState::UNKNOWN;
  uint32_t deviceId;
  string pubKey;
  string version;
  string revision;
  string chipId;
  string systemPath;
  bool nativeUSBSupport;
};


#endif