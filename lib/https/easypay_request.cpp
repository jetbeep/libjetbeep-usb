#include "./easypay_request.hpp"
#include "../utils/utils.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

using namespace std;
using namespace JetBeep;

string EasyPayAPI::tokenPaymentReqToJSON(TokenPaymentRequest& data) {
  pt::ptree json;

  // extract token, deviceId, signature from token
  auto tokenParts = Utils::splitString(data.PaymentTokenFull, ";");

  if (tokenParts.size() != 3) {
    throw runtime_error("invalid payment token"); // should never happen
  }

  string SignatureBox = tokenParts[2];
  uint32_t DeviceId = std::stoi(tokenParts[1]);
  string PaymentToken = tokenParts[0];

  // fill JSON
  json.put("Fields.DeviceId", "{{DeviceId}}");
  json.put("Fields.MerchantCashboxId", data.MerchantCashboxId);
  json.put("Fields.MerchantTransactionId", data.MerchantTransactionId);

  json.put("AmountInCoin", "{{AmountInCoin}}");
  json.put("DateRequest", data.DateRequest);
  json.put("SignatureMerchant", data.SignatureMerchant);
  json.put("SignatureBox", SignatureBox);
  json.put("PaymentToken", PaymentToken);

  for (std::pair<string, string> pair : data.Metadata) {
    json.put("Metadata." + pair.first, pair.second);
  }

  std::stringstream stream;
  pt::write_json(stream, json);

  string templateJson = stream.str();

  Utils::replaceInTemplate(templateJson, "{{DeviceId}}", std::to_string(DeviceId));
  Utils::replaceInTemplate(templateJson, "{{AmountInCoin}}", std::to_string(data.AmountInCoin));

  return templateJson;
}

string EasyPayAPI::tokenGetStatusReqToJSON(TokenGetStatusRequest& data) {
  pt::ptree json;
  json.put("DeviceId", "{{DeviceId}}");
  json.put("DateRequest", data.DateRequest);
  json.put("SignatureMerchant", data.SignatureMerchant);
  json.put("AmountInCoin", "{{AmountInCoin}}");
  json.put("MerchantTransactionId", data.MerchantTransactionId);

  std::stringstream stream;
  pt::write_json(stream, json);

  string templateJson = stream.str();

  Utils::replaceInTemplate(templateJson, "{{DeviceId}}", std::to_string(data.DeviceId));
  Utils::replaceInTemplate(templateJson, "{{AmountInCoin}}", std::to_string(data.AmountInCoin));

  return templateJson;
}

string EasyPayAPI::tokenRefundReqToJSON(TokenRefundRequest& data) {
  pt::ptree json;
  bool useUUID = data.PaymentRequestUid.length() > 0;
  json.put("DeviceId", "{{DeviceId}}");
  json.put("DateRequest", data.DateRequest);
  json.put("SignatureMerchant", data.SignatureMerchant);
  if (useUUID) {
    json.put("PaymentRequestUid", data.PaymentRequestUid);
  } else {
    json.put("TransactionId", "{{TransactionId}}");
  }

  std::stringstream stream;
  pt::write_json(stream, json);

  string templateJson = stream.str();

  Utils::replaceInTemplate(templateJson, "{{DeviceId}}", std::to_string(data.DeviceId));
  if (!useUUID) {
    Utils::replaceInTemplate(templateJson, "{{TransactionId}}", std::to_string(data.TransactionId));
  }

  return templateJson;
}