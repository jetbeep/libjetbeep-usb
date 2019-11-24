#include "./easypay_response.hpp"
#include "./http_errors.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

using namespace std;
using namespace JetBeep;

/*
unspecified error
200
{
    "model": {
        "_errors": [
            {
                "<Exception>k__BackingField": {
                    "ClassName": "Newtonsoft.Json.JsonSerializationException",
                    "Message": "Required property 'DeviceId' not found in JSON. Path '', line 8, position 1.",
                    "Data": null,
                    "InnerException": null,
                    "HelpURL": null,
                    "StackTraceString": "   at Newtonsoft.Json.Serialization.JsonSerializerInternalReader.EndProcessProperty(Object newObject, JsonReader reader, JsonObjectContract contract, Int32 initialDepth, JsonProperty property, PropertyPresence presence, Boolean setDefaultValue)",
                    "RemoteStackTraceString": null,
                    "RemoteStackIndex": 0,
                    "ExceptionMethod": "8\nEndProcessProperty\nNewtonsoft.Json, Version=12.0.0.0, Culture=neutral, PublicKeyToken=30ad4fe6b2a6aeed\nNewtonsoft.Json.Serialization.JsonSerializerInternalReader\nVoid EndProcessProperty(System.Object, Newtonsoft.Json.JsonReader, Newtonsoft.Json.Serialization.JsonObjectContract, Int32, Newtonsoft.Json.Serialization.JsonProperty, PropertyPresence, Boolean)",
                    "HResult": -2146233088,
                    "Source": "Newtonsoft.Json",
                    "WatsonBuckets": null
                },
                "<ErrorMessage>k__BackingField": ""
            }
        ],
        "<Value>k__BackingField": null
    }
}
specifiend error
{
    "Result": null,
    "Uid": "fd3034b0-0940-4f54-911b-c86f2d5490e4",
    "Errors": [
        {
            "Error": "service",
            "CodeId": 11021,
            "CodeName": "WebApi_Invalid_Field_SignatureMerchant",
            "ErrorMessage": null,
            "UserMessage": "Ошибка проверки SignatureMerchant",
            "Reason": null
        }
    ]
}
*/

/*
{
  "Result": {
    "Status": "string",
    "TransactionId": 0,
    "TransactionDatePost": "2018-06-26T06:53:03.691Z",
    "PaymentRequestUid": "00000000-0000-0000-0000-000000000000",
    "MerchantTransactionId": "string",
  },
  "Uid": "00000000-0000-0000-0000-000000000000",
  "Errors": null
}
*/

static EasyPayAPI::PaymentStatus parseStatusString(string& status) {
  if (status == "None") {
    return EasyPayAPI::PaymentStatus::None;
  }
  if (status == "Inserted") {
    return EasyPayAPI::PaymentStatus::Inserted;
  }
  if (status == "Accepted") {
    return EasyPayAPI::PaymentStatus::Accepted;
  }
  if (status == "Declined") {
    return EasyPayAPI::PaymentStatus::Declined;
  }
  if (status == "Deleted") {
    return EasyPayAPI::PaymentStatus::Deleted;
  }
  if (status == "InProcess") {
    return EasyPayAPI::PaymentStatus::InProcess;
  }
  if (status == "Hold") {
    return EasyPayAPI::PaymentStatus::Hold;
  }
  if (status == "Created") {
    return EasyPayAPI::PaymentStatus::Created;
  }
  return EasyPayAPI::PaymentStatus::None;
}

static void parseErrors(const pt::ptree* parser, EasyPayAPI::EasyPayResult* result) {
  string emptyStr = "";
  for (const pt::ptree::value_type& errorObj : parser->get_child("Errors")) {
    if (!errorObj.first.empty()) {
      // invalid list
      throw HttpErrors::APIError();
    }
    EasyPayAPI::EasyPayError errorStruct;
    auto field = errorObj.second.get_child_optional("Error");
    if (field.has_value() && field.value().data() != "null") {
      errorStruct.Error = field.value().data();
    }
    field = errorObj.second.get_child_optional("CodeId");
    if (field.has_value() && field.value().data() != "null") {
      errorStruct.CodeId = std::stoi(field.value().data());
    }
    field = errorObj.second.get_child_optional("CodeName");
    if (field.has_value() && field.value().data() != "null") {
      errorStruct.CodeName = field.value().data();
    }
    field = errorObj.second.get_child_optional("ErrorMessage");
    if (field.has_value() && field.value().data() != "null") {
      errorStruct.ErrorMessage = field.value().data();
    }
    field = errorObj.second.get_child_optional("UserMessage");
    if (field.has_value() && field.value().data() != "null") {
      errorStruct.UserMessage = field.value().data();
    }
    field = errorObj.second.get_child_optional("Reason");
    if (field.has_value() && field.value().data() != "null") {
      errorStruct.Reason = field.value().data();
    }
    result->Errors.push_back(errorStruct);
  }
  if (!result->Errors.empty()) {
    result->primaryErrorMsg = result->Errors[0].UserMessage;
  }
}

static void parseResult(const pt::ptree* parser, EasyPayAPI::EasyPayResult* result) {
  auto node = parser->get_child_optional("Result.Status");
  if (node.has_value()) {
    auto statusStr = parser->get<string>("Result.Status");
    result->Status = parseStatusString(statusStr);
  }

  node = parser->get_child_optional("Result.TransactionId");
  if (node.has_value()) {
    result->TransactionId = parser->get<long>("Result.TransactionId");
  }

  node = parser->get_child_optional("Result.TransactionDatePost");
  if (node.has_value()) {
    result->TransactionDatePost = parser->get<string>("Result.TransactionDatePost");
  }

  node = parser->get_child_optional("Result.PaymentRequestUid");
  if (node.has_value()) {
    result->PaymentRequestUid = parser->get<string>("Result.PaymentRequestUid");
  }

  node = parser->get_child_optional("Result.MerchantTransactionId");
  if (node.has_value()) {
    result->MerchantTransactionId = parser->get<string>("Result.MerchantTransactionId");
  }
}

static void parseResponse(const string& json, pt::ptree* parser, EasyPayAPI::EasyPayResult* result) {
  stringstream stream = stringstream(json);
  bool hasResult = false;
  bool hasErrors = false;
  try {
    pt::read_json(stream, *parser);
    // check form unspecified error case
    result->Uid = parser->get<string>("Uid");
    auto node = parser->get_child_optional("Result");
    if (node.has_value()) {
      hasResult = node.value().data() != "null";
    }
    node = parser->get_child_optional("Errors");
    if (node.has_value()) {
      hasErrors = node.value().data() != "null";
    }

    if (!hasErrors && !hasResult) {
      throw HttpErrors::APIError();
    }

  } catch (... /*pt::ptree_bad_path &er*/) {
    throw HttpErrors::APIError();
  }

  if (hasResult) {
    parseResult(parser, result);
  }
  if (hasErrors) {
    parseErrors(parser, result);
  }
}

EasyPayAPI::EasyPayResult EasyPayAPI::parseTokenPaymentResult(const string& json) {
  pt::ptree parser;
  EasyPayResult result;
  parseResponse(json, &parser, &result);
  return result;
};

/*
{
  "Result": {
    "Status": "string",
    "TransactionId": 0
   },
  "Uid": "00000000-0000-0000-0000-000000000000",
  "Errors": null
}
*/

EasyPayAPI::EasyPayResult EasyPayAPI::parseTokenRefundResult(const string& json) {
  pt::ptree parser;
  EasyPayResult result;
  parseResponse(json, &parser, &result);
  return result;
};

/*
{
  "Result": {
    "Status": "string",
    "TransactionId": 0,
    "TransactionDate": "2018-06-26T06:53:03.691Z"
  },
  "Uid": "00000000-0000-0000-0000-000000000000",
  "Errors": null
}
*/
EasyPayAPI::EasyPayResult EasyPayAPI::parseTokenGetStatusResult(const string& json) {
  pt::ptree parser;
  EasyPayResult result;
  parseResponse(json, &parser, &result);
  return result;
};
