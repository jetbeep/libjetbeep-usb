#ifndef JETBEEP_DEVICE_TYPES_H
#define JETBEEP_DEVICE_TYPES_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <exception>

#include "barcode.hpp"
#include "payment_error.hpp"
#include "device_errors.hpp"

namespace JetBeep {  
  enum class SerialMobileEvent {
    connected,
    disconnected
  };

  typedef struct SerialGetStateResult {
    bool isSessionOpened;
    bool isBarcodesRequested;
    bool isPaymentCreated;
    bool isWaitingForPaymentConfirmation;
    bool isRefundRequested;
  } SerialGetStateResult;

  enum class SerialBeginPrivateMode {
    setup,
    config
  };

  typedef std::function<void(std::exception_ptr)> SerialErrorCallback;
  typedef std::function<void(const std::vector<Barcode> &)> SerialBarcodesCallback;
  typedef std::function<void(const PaymentError &)> SerialPaymentErrorCallback;
  typedef std::function<void()> SerialPaymentSuccessCallback;
  typedef std::function<void(const std::string &)> SerialPaymentTokenCallback;
  typedef std::function<void(const SerialMobileEvent &)> SerialMobileCallback;

  typedef std::unordered_map<std::string, std::string> PaymentMetadata;
}

#endif