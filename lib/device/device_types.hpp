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
  enum class SerialError {
    noError,
    ioError,
    protocolError
  };
  
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

  typedef std::function<void(const SerialError &)> SerialErrorCallback;
  typedef std::function<void(const std::vector<Barcode> &)> SerialBarcodesCallback;
  typedef std::function<void(const PaymentError &)> SerialPaymentErrorCallback;
  typedef std::function<void()> SerialPaymentSuccessCallback;
  typedef std::function<void(const std::string &)> SerialPaymentTokenCallback;
  typedef std::function<void(const SerialMobileEvent &)> SerialMobileCallback;
  typedef std::function<void(const std::string&)> SerialGetCallback;
  typedef std::function<void(const SerialGetStateResult& )> SerialGetStateCallback;

  typedef std::unordered_map<std::string, std::string> PaymentMetadata;
}

#endif