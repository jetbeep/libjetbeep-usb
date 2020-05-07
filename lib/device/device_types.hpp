#ifndef JETBEEP_DEVICE_TYPES_H
#define JETBEEP_DEVICE_TYPES_H

#include <exception>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "barcode.hpp"
#include "device_errors.hpp"
#include "payment_error.hpp"
#include "nfc/nfc_types.hpp"

#define INTERFACE_ENABLED  "enabled"
#define INTERFACE_DISABLED "disabled"

namespace JetBeep {
  enum class SerialMobileEvent { connected, disconnected };

  typedef struct SerialGetStateResult {
    bool isSessionOpened;
    bool isBarcodesRequested;
    bool isPaymentCreated;
    bool isWaitingForPaymentConfirmation;
    bool isRefundRequested;
  } SerialGetStateResult;

  enum class SerialBeginPrivateMode { setup, config };

  typedef std::function<void(std::exception_ptr)> SerialErrorCallback;
  typedef std::function<void(const std::vector<Barcode>&)> SerialBarcodesCallback;
  typedef std::function<void(const PaymentError&)> SerialPaymentErrorCallback;
  typedef std::function<void()> SerialPaymentSuccessCallback;
  typedef std::function<void(const std::string&)> SerialPaymentTokenCallback;
  typedef std::function<void(const SerialMobileEvent&)> SerialMobileCallback;
  typedef std::function<void(const SerialNFCEvent&, const NFC::DetectionEventData&)> SerialNFCEventCallback;
  typedef std::function<void(const NFC::DetectionErrorReason&)> SerialNFCDetectionErrorCallback;

  typedef std::unordered_map<std::string, std::string> PaymentMetadata;

  enum class DeviceOperationMode { scanner, driver };

  enum class DeviceConnectionRole { master, slave };

} // namespace JetBeep

#endif