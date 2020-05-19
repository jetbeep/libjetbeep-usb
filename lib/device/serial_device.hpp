#ifndef JETBEEP_DEVICE__H
#define JETBEEP_DEVICE__H

#define JETBEEP_DEVICE_MIN_FW_VER "1.0.4-beta"

#include "../io/iocontext.hpp"
#include "../utils/promise.hpp"
#include "device_parameter.hpp"
#include "device_types.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace JetBeep {
  class SerialDevice {
  public:
    SerialDevice(IOContext context = IOContext::context);
    virtual ~SerialDevice();

    void open(const std::string& path);
    void close();

    Promise<void> openSession();
    Promise<void> closeSession();
    Promise<void> requestBarcodes();
    Promise<void> cancelBarcodes();
    Promise<void> createPayment(uint32_t amount,
                                const std::string& transactionId,
                                const std::string& cashierId = "",
                                const PaymentMetadata& metadata = PaymentMetadata());
    Promise<void> createPaymentToken(uint32_t amount,
                                     const std::string& transactionId,
                                     const std::string& cashierId = "",
                                     const PaymentMetadata& metadata = PaymentMetadata());
    Promise<void> confirmPayment();
    Promise<void> cancelPayment();
    Promise<void> resetState();
    Promise<std::string> get(const DeviceParameter& parameter);
    Promise<void> set(const DeviceParameter& parameter, const std::string& value);
    Promise<void> beginPrivate(const SerialBeginPrivateMode& mode);
    Promise<void> commit(const std::string& signature);
    Promise<SerialGetStateResult> getState();

    /* NFC cmds */
    Promise<std::string> nfcReadMFC(uint8_t blockNo);
    Promise<std::string> nfcSecureReadMFC(uint8_t blockNo,
                                          const std::string& keyBase64,
                                          const std::string& keyType);

    Promise<void> nfcWriteMFC(uint8_t blockNo, const std::string& contentBase64);
    Promise<void> nfcSecureWriteMFC(uint8_t blockNo,
                                    const std::string& contentBase64,
                                    const std::string& keyBase64,
                                    const std::string& keyType);

    SerialErrorCallback errorCallback;
    SerialBarcodesCallback barcodesCallback;
    SerialPaymentErrorCallback paymentErrorCallback;
    SerialPaymentSuccessCallback paymentSuccessCallback;
    SerialPaymentTokenCallback paymentTokenCallback;
    SerialMobileCallback mobileCallback;
    SerialNFCEventCallback nfcEventCallback;
    SerialNFCDetectionErrorCallback nfcDetectionErrorCallback; 
  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
  };
} // namespace JetBeep

#endif