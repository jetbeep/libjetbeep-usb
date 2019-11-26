#include "./easypay_request.hpp"
#include "../utils/utils.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

using namespace std;
using namespace JetBeep;

string EasyPayAPI::tokenPaymentReqToJSON(TokenPaymentRequest& data) {
  pt::ptree json;

  //extract token, deviceId, signature from token
  auto tokenParts = Utils::splitString(data.PaymentTokenFull, ";");

  if (tokenParts.size() != 3) {
    throw runtime_error("invalid payment token"); // should never happen
  }

  string SignatureBox = tokenParts[2];
  uint32_t DeviceId = std::stoi(tokenParts[1]);
  string PaymentToken = tokenParts[0];

  //fill JSON
  json.put("Fields.DeviceId", DeviceId);
  json.put("Fields.MerchantCashboxId", data.MerchantCashboxId);
  json.put("Fields.MerchantTransactionId", data.MerchantTransactionId);
  json.put("AmountInCoin", data.AmountInCoin);
  json.put("DateRequest", data.DateRequest);
  json.put("SignatureMerchant", data.SignatureMerchant);
  json.put("SignatureBox", SignatureBox);
  json.put("PaymentToken", PaymentToken);

  std::stringstream stream;
  pt::write_json(stream, json);

  return stream.str();
}

string EasyPayAPI::tokenGetStatusReqToJSON(TokenGetStatusRequest& data) {
  pt::ptree json;
  json.put("DeviceId", data.DeviceId);
  json.put("DateRequest", data.DateRequest);
  json.put("SignatureMerchant", data.SignatureMerchant);
  json.put("AmountInCoin", data.AmountInCoin);
  json.put("MerchantTransactionId", data.MerchantTransactionId);

  std::stringstream stream;
  pt::write_json(stream, json);

  return stream.str();
}

string EasyPayAPI::tokenRefundReqToJSON(TokenRefundRequest& data) {
  pt::ptree json;
  json.put("DeviceId", data.DeviceId);
  json.put("DateRequest", data.DateRequest);
  json.put("SignatureMerchant", data.SignatureMerchant);
  json.put("TransactionId", data.TransactionId);

  std::stringstream stream;
  pt::write_json(stream, json);

  return stream.str();
}