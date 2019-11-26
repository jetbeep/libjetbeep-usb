#ifndef JETBEEP_AUTODEVICE__H
#define JETBEEP_AUTODEVICE__H

#include <exception>
#include <functional>
#include <memory>
#include <string>

#include "../io/iocontext.hpp"
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

  typedef std::function<void(const PaymentError& error)> AutoDevicePaymentErrorCallback;
  typedef std::function<void(AutoDeviceState state, std::exception_ptr error)> AutoDeviceStateCallback;
  typedef SerialMobileCallback AutoDeviceMobileCallback;

  class AutoDevice {
  public:
    AutoDevice(IOContext context = IOContext::context);
    virtual ~AutoDevice();

    void start();
    void stop();
    void openSession();
    void closeSession();

    Promise<std::vector<Barcode>> requestBarcodes();
    void cancelBarcodes();

    Promise<void> createPayment(uint32_t amount,
                                const std::string& transactionId,
                                const std::string& cashierId = "",
                                const PaymentMetadata& metadata = PaymentMetadata());
    void confirmPayment();

    Promise<std::string> createPaymentToken(uint32_t amount,
                                            const std::string& transactionId,
                                            const std::string& cashierId = "",
                                            const PaymentMetadata& metadata = PaymentMetadata());
    void cancelPayment();

    bool isMobileConnected();

    void* opaque;

    AutoDeviceStateCallback stateCallback;
    AutoDevicePaymentErrorCallback paymentErrorCallback;
    AutoDeviceMobileCallback mobileCallback;

    AutoDeviceState state();

  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
  };
} // namespace JetBeep

#endif