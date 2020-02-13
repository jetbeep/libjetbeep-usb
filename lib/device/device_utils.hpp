#ifndef JETBEEP_DEVICE_UTILS__H
#define JETBEEP_DEVICE_UTILS__H

#include "device_parameter.hpp"
#include "device_types.hpp"
#include <string>
#include <vector>

namespace JetBeep {
  class DeviceUtils {
  public:
    static std::string parameterToString(const DeviceParameter& parameter);
    static DeviceParameter stringToParameter(const std::string& parameter);

    static std::string operationModeToString(const DeviceOperationMode& value);
    static DeviceOperationMode stringToOperationMode(const std::string& value);

    static std::string connectionRoleToString(const DeviceConnectionRole& value);
    static DeviceConnectionRole stringToConnectionRole(const std::string& value);

    static std::string boolToDeviceBoolStr(bool value) {
      return value ? "1" : "0";
    };

    static std::string mobileAppsUUIDsToString(std::vector<uint32_t> list);
  };
} // namespace JetBeep

namespace DeviceResponses {
  // responses
  const std::string openSession = "OPEN_SESSION";
  const std::string closeSession = "CLOSE_SESSION";
  const std::string requestBarcodes = "REQUEST_BARCODES";
  const std::string cancelBarcodes = "CANCEL_BARCODES";
  const std::string createPayment = "CREATE_PAYMENT";
  const std::string cancelPayment = "CANCEL_PAYMENT";
  const std::string confirmPayment = "CONFIRM_PAYMENT";
  const std::string createPaymentToken = "CREATE_PAYMENT_TOKEN";
  const std::string resetState = "RESET_STATE";
  const std::string get = "GET";
  const std::string set = "SET";
  const std::string beginPrivate = "BEGIN_PRIVATE";
  const std::string commit = "COMMIT";
  const std::string getState = "GETSTATE";

  // events
  const std::string mobileConnected = "MOBILE_CONNECTED";
  const std::string mobileDisconnected = "MOBILE_DISCONNECTED";
  const std::string barcodes = "BARCODES";
  const std::string paymentSuccessful = "PAYMENT_SUCCESSFUL";
  const std::string paymentError = "PAYMENT_ERROR";
  const std::string paymentToken = "PAYMENT_TOKEN";
} // namespace DeviceResponses

#endif