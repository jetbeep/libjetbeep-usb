#ifndef JETBEEP_AUTODEVICE__H
#define JETBEEP_AUTODEVICE__H

#include <exception>
#include <functional>
#include <memory>
#include <string>

#include "../io/iocontext.hpp"
#include "../utils/promise.hpp"
#include "device_types.hpp"
#include "./nfc/mifare-classic/mfc-provider.hpp"
#include "./nfc/nfc-api-provider.hpp"

namespace JetBeep {
  enum class AutoDeviceState {
    invalid = 0, // if you change here, don't forget to change it in ALL bindings: C, Java, Delphi, etc
    firmwareVersionNotSupported,

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
  typedef SerialNFCEventCallback AutoDeviceNFCEventCallback;
  typedef SerialNFCDetectionErrorCallback AutoDeviceNFCDetectionErrorCallback;

  class AutoDevice {
  public:
    AutoDevice(IOContext context = IOContext::context);
    virtual ~AutoDevice();

    void start();
    void stop();
    void openSession();
    void closeSession();

    void enableBluetooth();
    void disableBluetooth();

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

    std::string version();
    unsigned long deviceId();

    void* opaque;

    AutoDeviceStateCallback stateCallback;
    AutoDevicePaymentErrorCallback paymentErrorCallback;
    AutoDeviceMobileCallback mobileCallback;

    AutoDeviceState state();

    /* NFC related section */
    AutoDeviceNFCEventCallback nfcEventCallback;
    AutoDeviceNFCDetectionErrorCallback nfcDetectionErrorCallback;

    void enableNFC();
    void disableNFC();

    bool isNFCDetected();
    NFC::DetectionEventData getNFCCardInfo();

    NFC::MifareClassic::MifareClassicProvider getNFCMifareApiProvider();

  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
  };
} // namespace JetBeep

#endif