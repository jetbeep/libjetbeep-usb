#include "./portal_response.hpp"
#include "./http_errors.hpp"
#include "../device/device_utils.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

using namespace std;
using namespace JetBeep;
using namespace JetBeep::PortalAPI;

/*
{
    "shopId": 5,
    "mode": "driver",
    "txPower": 0,
    "tapSensitivity": -35,
    "phoneConFeedback": true,
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
    "virtKeyboard": "0 us no_ending no_alt"
    "configVersion": 2
}
*/

DeviceConfig PortalAPI::parseDeviceConfigResult(const string& json) {
  DeviceConfig result;
  pt::ptree parser;
  stringstream stream = stringstream(json);
  try {
    pt::read_json(stream, parser);

    auto node = parser.get_child_optional("shopId");
    if (node.has_value()) {
      string value = node.value().data();
      result.shopId = value == "null" ? 0 : (uint32_t)std::stoul(value);
    }
    node = parser.get_child_optional("mode");
    if (node.has_value()) {
      result.mode = DeviceUtils::stringToOperationMode(node.value().data());
    }
    node = parser.get_child_optional("txPower");
    if (node.has_value()) {
      result.txPower = std::stoi(node.value().data());
    }
    node = parser.get_child_optional("tapSensitivity");
    if (node.has_value()) {
      result.tapSensitivity = std::stoi(node.value().data());
    }
    node = parser.get_child_optional("phoneConFeedback");
    if (node.has_value()) {
      result.phoneConFeedback = node.value().data() == "true" ? true : false;
    }
    node = parser.get_child_optional("devEnv");
    if (node.has_value()) {
      result.devEnv = node.value().data() == "true" ? true : false;
    }
    node = parser.get_child_optional("connectionRole");
    if (node.has_value()) {
      result.connectionRole = DeviceUtils::stringToConnectionRole(node.value().data());
    }
    node = parser.get_child_optional("logLevel");
    if (node.has_value()) {
      result.logLevel = std::stoi(node.value().data());
    }
    node = parser.get_child_optional("merchantId");
    if (node.has_value()) {
      string value = node.value().data();
      result.merchantId = value == "null" ? 0 : (uint16_t)std::stoul(value);
    }
    node = parser.get_child_optional("domainShopId");
    if (node.has_value()) {
      string value = node.value().data();
      result.domainShopId = value == "null" ? 0 : (uint16_t)std::stoul(value);
    }
    node = parser.get_child_optional("deviceId");
    if (node.has_value()) {
      result.deviceId = (uint32_t)std::stoul(node.value().data());
    }
    node = parser.get_child_optional("cashierId");
    if (node.has_value()) {
      string value = node.value().data();
      result.cashierId = value == "null" ? "" : value;
    }
    node = parser.get_child_optional("shopKey");
    if (node.has_value()) {
      string value = node.value().data();
      result.shopKey = value == "null" ? "" : value;
    }
    node = parser.get_child_optional("mobileAppsUUIDs");
    if (node.has_value()) {
      for (const pt::ptree::value_type& appIdNode : node.get() ) {
        result.mobileAppsUUIDs.push_back(appIdNode.second.get_value<uint32_t>());
      }
    }
    node = parser.get_child_optional("serialNumber");
    if (node.has_value()) {
      result.serialNumber = node.value().data();
    }
    node = parser.get_child_optional("signature");
    if (node.has_value()) {
      result.signature = node.value().data();
    }
    node = parser.get_child_optional("signatureType");
    if (node.has_value()) {
      result.signatureType = node.value().data();
    }
    node = parser.get_child_optional("virtKeyboard");
    if (node.has_value()) {
      string value = node.value().data();
      result.virtKeyboard = value == "null" ? "" : value;
    }
    node = parser.get_child_optional("configVersion");
    if (node.has_value()) {
      result.configVersion = std::stoi(node.value().data());
    }
  } /*catch (pt::json_parser::json_parser_error& jsonError) {
    cout << "JSON error: " << jsonError.what() << "\n\n";
    throw HttpErrors::APIError();
  } */
  catch (...) {
    throw HttpErrors::APIError();
  }

  return result;
};

/*
TODO parse validation errors
{
    "errors": {
        "fwVersion": [
            "Fw version is too long (maximum is 14 characters)"
        ],
        ...[string]: [array<string>]
    },
    "errorCode": "STRING"
}
*/
