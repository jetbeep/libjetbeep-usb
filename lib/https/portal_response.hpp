#ifndef PORTAL_RESPONSE
#define PORTAL_RESPONSE

#include <string>
#include <vector>
#include <cstdint>

#include "../device/device_types.hpp"
#include "./https_response.hpp"

using namespace std;

namespace JetBeep::PortalAPI {

  class DeviceConfig {
  public:
    uint32_t shopId;
    DeviceOperationMode mode;
    int8_t txPower;
    int8_t tapSensitivity;
    bool phoneConFeedback;
    bool devEnv;
    DeviceConnectionRole connectionRole;
    uint8_t logLevel;
    uint16_t merchantId;
    uint16_t domainShopId;
    uint32_t deviceId;
    string cashierId;
    string shopKey; 
    vector<uint32_t> mobileAppsUUIDs;
    string serialNumber;
    string signature;
    string signatureType;
    uint8_t configVersion;
  };

  class DeviceConfigResponse : public HTTPResponseBase {
  public:
    DeviceConfig config;
  };

  DeviceConfig parseDeviceConfigResult(const string &json);
  
} // namespace JetBeep::PortalAPI

#endif