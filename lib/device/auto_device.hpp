#ifndef JETBEEP_AUTODEVICE__H
#define JETBEEP_AUTODEVICE__H

#include <string>
#include <functional>
#include <memory>
#include <exception>

#include "../utils/promise.hpp"
#include "device_types.hpp"

namespace JetBeep {
  enum class AutoDeviceState {
    invalid,
    sessionOpened,
    sessionClosed,
    waitingForBarcodes,

    waitingForPaymentResult,
    waitingForConfirmation,

    waitingForPaymentToken
  };

  typedef std::function<void (const PaymentError& error)> AutoDevicePaymentErrorCallback;
  typedef std::function<void (AutoDeviceState state, std::exception_ptr error)> AutoDeviceStateCallback;

  class AutoDevice {
  public:
    AutoDevice();
    virtual ~AutoDevice();

    void start();
    void stop();
    void openSession();
    void closeSession();

    Promise<std::vector<Barcode>> requestBarcodes();
    void cancelBarcodes();

    Promise<void> createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", 
      const PaymentMetadata& metadata = PaymentMetadata());    
    void confirmPayment();

    Promise<std::string> createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", 
      const PaymentMetadata& metadata = PaymentMetadata());
    void cancelPayment();
    
    AutoDeviceStateCallback stateCallback;
    AutoDevicePaymentErrorCallback paymentErrorCallback;

    AutoDeviceState state();
  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
  };
}

#endif