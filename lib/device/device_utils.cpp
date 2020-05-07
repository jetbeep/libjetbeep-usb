#include "device_utils.hpp"
#include <stdexcept>
#include "../utils/utils.hpp"
#include <string>
#include <sstream>
#include <boost/range.hpp>

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
  case DeviceParameter::virtKeyboard:
    return "virtKeyboard";
  case DeviceParameter::nfc:
    return "nfc";
  case DeviceParameter::bluetooth:
    return "bluetooth";
  default:
    throw runtime_error("invalid device parameter");
    break;
  }
}

DeviceParameter DeviceUtils::stringToParameter(const std::string& parameter) {
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
  } else if (parameter == "virtKeyboard") {
    return DeviceParameter::virtKeyboard;
  } else if (parameter == "bluetooth") {
    return DeviceParameter::bluetooth;
  } else if (parameter == "nfc") {
    return DeviceParameter::nfc;
  } else {
    throw runtime_error("invalid input string");
  }
}

std::string DeviceUtils::operationModeToString(const DeviceOperationMode& value) {
  switch (value) {
  case DeviceOperationMode::driver:
    return "driver";
  case DeviceOperationMode::scanner:
    return "scanner";
  default:
    throw runtime_error("invalid DeviceOperationMode");
    break;
  }
}

DeviceOperationMode DeviceUtils::stringToOperationMode(const std::string& value) {
  if (value == "driver") {
    return DeviceOperationMode::driver;
  } else if (value == "scanner") {
    return DeviceOperationMode::scanner;
  } else {
    throw runtime_error("invalid input string");
  }
}

std::string DeviceUtils::connectionRoleToString(const DeviceConnectionRole& value) {
  switch (value) {
  case DeviceConnectionRole::master:
    return "master";
  case DeviceConnectionRole::slave:
    return "slave";
  default:
    throw runtime_error("invalid DeviceConnectionRole");
    break;
  }
}

DeviceConnectionRole DeviceUtils::stringToConnectionRole(const std::string& value) {
  if (value == "master") {
    return DeviceConnectionRole::master;
  } else if (value == "slave") {
    return DeviceConnectionRole::slave;
  } else {
    throw runtime_error("invalid input string");
  }
}

std::string DeviceUtils::mobileAppsUUIDsToString(std::vector<uint32_t> list) {
  if (list.size() == 0) {
    return "";
  }
  std::stringstream stream;

  stream << Utils::numberToHexString(list.at(0));

  for (auto uid : boost::make_iterator_range(list.begin() + 1, list.end())) {
    stream << " " << Utils::numberToHexString(uid);
  }

  return stream.str();
};


NFC::DetectionEventData DeviceUtils::parseNFCDetectionEventData(const vector<string>& params) {
  if (params.size() != 2) {
    throw runtime_error("invalid NFC detection params count");
  }
  NFC::DetectionEventData eventData;
  int typeId = std::stoi(params[0]);
  switch (typeId) {
    case 1: eventData.cardType = NFC::CardType::EMV_CARD; break;
    case 2: eventData.cardType = NFC::CardType::MIFARE_CLASSIC_1K; break;
    case 3: eventData.cardType = NFC::CardType::MIFARE_CLASSIC_4K; break;
    case 4: eventData.cardType = NFC::CardType::MIFARE_PLUS_2K; break;
    case 5: eventData.cardType = NFC::CardType::MIFARE_PLUS_4K; break;
    case 6: eventData.cardType = NFC::CardType::MIFARE_DESFIRE_2K; break;
    case 7: eventData.cardType = NFC::CardType::MIFARE_DESFIRE_4K; break;
    case 8: eventData.cardType = NFC::CardType::MIFARE_DESFIRE_8K; break;
    default: 
      eventData.cardType = NFC::CardType::UNKNOWN;
  }
  eventData.meta = params[1];
  return eventData;
}