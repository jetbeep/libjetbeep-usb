#include "device_utils.hpp"
#include <stdexcept>

using namespace JetBeep;
using namespace std;

std::string DeviceUtils::parameterToString(const DeviceParameter& parameter) {
  switch (parameter) {
  case DeviceParameter::version:
    return "version";
  case DeviceParameter::shopId:
    return "shopId";
  case DeviceParameter::deviceId:
    return "deviceId";
  case DeviceParameter::mode:
    return "mode";
  case DeviceParameter::pubKey:
    return "pubKey";
  case DeviceParameter::chipId:
    return "chipId";
  case DeviceParameter::revision:
    return "revision";
  case DeviceParameter::paymentPubKey:
    return "paymentPubKey";
  case DeviceParameter::shopKey:
    return "shopKey";
  case DeviceParameter::cashierId:
    return "cashierId";
  case DeviceParameter::txPower:
    return "txPower";
  case DeviceParameter::tapSensitivity:
    return "tapSensitivity";
  case DeviceParameter::phoneConFeedback:
    return "phoneConFeedback";
  case DeviceParameter::proximitySensitivity:
    return "proximitySensitivity";
  case DeviceParameter::mobileAppsUUIDs:
    return "mobileAppsUUIDs";
  case DeviceParameter::mac:
    return "mac";
  case DeviceParameter::devEnv:
    return "devEnv";
  case DeviceParameter::connectionRole:
    return "connectionRole";
  case DeviceParameter::logLevel:
    return "logLevel";
  case DeviceParameter::merchantId:
    return "merchantId";
  case DeviceParameter::domainShopId:
    return "domainShopId";
  default:
    throw runtime_error("invalid device parameter");
    break;
  }
}

DeviceParameter DeviceUtils::stringToParameter(const std::string &parameter) {
  if (parameter == "version") {
    return DeviceParameter::version;
  } else if (parameter == "shopId") {
    return DeviceParameter::shopId;
  } else if (parameter == "deviceId") {
    return DeviceParameter::deviceId;
  } else if (parameter == "mode") {
    return DeviceParameter::mode;
  } else if (parameter == "pubKey") {
    return DeviceParameter::pubKey;
  } else if (parameter == "chipId") {
    return DeviceParameter::chipId;
  } else if (parameter == "revision") {
    return DeviceParameter::revision;
  } else if (parameter == "paymentPubKey") {
    return DeviceParameter::paymentPubKey;
  } else if (parameter == "shopKey") {
    return DeviceParameter::shopKey;
  } else if (parameter == "cashierId") {
    return DeviceParameter::cashierId;
  } else if (parameter == "txPower") {
    return DeviceParameter::txPower;
  } else if (parameter == "tapSensitivity") {
    return DeviceParameter::tapSensitivity;
  } else if (parameter == "phoneConFeedback") {
    return DeviceParameter::phoneConFeedback;
  } else if (parameter == "proximitySensitivity") {
    return DeviceParameter::proximitySensitivity;
  } else if (parameter == "mobileAppsUUIDs") {
    return DeviceParameter::mobileAppsUUIDs;
  } else if (parameter == "mac") {
    return DeviceParameter::mac;
  } else if (parameter == "devEnv") {
    return DeviceParameter::devEnv;
  } else if (parameter == "connectionRole") {
    return DeviceParameter::connectionRole;
  } else if (parameter == "logLevel") {
    return DeviceParameter::logLevel;
  } else if (parameter == "merchantId") {
    return DeviceParameter::merchantId;
  } else if (parameter == "domainShopId") {
    return DeviceParameter::domainShopId;
  } else {
    throw runtime_error("invalid input string");
  }
}