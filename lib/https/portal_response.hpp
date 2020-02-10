#ifndef EASYPAY_RESPONSE
#define EASYPAY_RESPONSE

#include <string>
#include <vector>
#include <cstdint>

#include "../device/device_types.hpp"

using namespace std;

namespace JetBeep::PortalAPI {

/*
{
    "shopId": 5,
    "mode": "driver",
    "txPower": 0,
    "tapSensitivity": -35,
    "phoneConFeedback": true,
    "proximitySensitivity": -70,
    "devEnv": true,
    "connectionRole": "master",
    "logLevel": 2,
    "merchantId": 3,
    "domainShopId": 1,
    "deviceId": 92,
    "cashierId": "Yevhenii_dev_board",
    "shopKey": "4e1e94cbca7ea7836269ce91ee572243684499c4e43e951bfd41221f1ea741ff",
    "mobileAppsUUIDs": [
        6043,
        6043
    ],
    "serialNumber": "A020100000092",
    "signature": "d3c30ba907a4d6aa1bda50fd5f0cebd1e8941994f16a7e5f57b29e8ffe7fd8428a1ac0fffab33dd9547bddd4a023a6ee76950cf836bbd9024aef4597a50a60aa",
    "signatureType": "config",
    "configVersion": 2
}
*/
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

  DeviceConfig parseDeviceConfigResult(const string &json);
  
} // namespace JetBeep::PortalAPI

#endif