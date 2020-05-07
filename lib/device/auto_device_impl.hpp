#ifndef JETBEEP_AUTO_DEVICE_IMPL__H
#define JETBEEP_AUTO_DEVICE_IMPL__H

#include "../detection/detection.hpp"
#include "../utils/logger.hpp"
#include "../utils/promise.hpp"
#include "auto_device.hpp"
#include "serial_device.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace JetBeep {
  class AutoDevice::Impl {
  public:
    Impl(AutoDeviceStateCallback* stateCallback,
         AutoDevicePaymentErrorCallback* paymentErrorCallback,
         AutoDeviceMobileCallback* mobileCallback,
         AutoDeviceNFCEventCallback*  nfcEventCallback,
         AutoDeviceNFCDetectionErrorCallback * nfcDetectionErrorCallback,
         IOContext context);
    virtual ~Impl();

    void start();
    void stop();

    void openSession();
    void closeSession();

    void enableBluetooth();
    void disableBluetooth();

    void enableNFC();
    void disableNFC();

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

    AutoDeviceState state();
    bool isMobileConnected();
    bool isNFCDetected();
    NFC::DetectionEventData getNFCCardInfo();
    std::string version();
    unsigned long deviceId();
    std::shared_ptr<NFC::MifareClassic::MifareClassicProvider> createMifareClassicProvider();

  private:
    IOContext m_context;
    bool m_started;
    bool m_mobileConnected;
    bool m_nfcDetected;

    NFC::DetectionEventData m_nfcCardInfo;

    AutoDeviceStateCallback* m_stateCallback;
    AutoDevicePaymentErrorCallback* m_paymentErrorCallback;
    AutoDeviceMobileCallback* m_mobileCallback;
    AutoDeviceNFCEventCallback*  m_nfcEventCallback;
    AutoDeviceNFCDetectionErrorCallback* m_nfcDetectionErrorCallback;

    Promise<std::vector<Barcode>> m_barcodesPromise;
    Promise<void> m_paymentPromise;
    Promise<std::string> m_paymentTokenPromise;
    DeviceCandidate m_candidate;
    Logger m_log;
    AutoDeviceState m_state;
    DeviceDetection m_detection;
    SerialDevice m_device;
    boost::asio::deadline_timer m_timer;
    std::recursive_mutex m_mutex;
    std::vector<std::function<void()>> m_pendingOperations;
    std::string m_version;
    unsigned long m_deviceId;

    void onDeviceEvent(DeviceDetectionEvent event, DeviceCandidate candidate);
    void changeState(AutoDeviceState state, std::exception_ptr exception = nullptr);
    void resetState();
    void initDevice();
    void handleTimeout(const boost::system::error_code& err);
    void handleInitError(const boost::system::error_code& err);
    void executeNextOperation();
    void enqueueOperation(const std::function<void()>& callback);
    void onBarcodes(const std::vector<Barcode>& barcodes);
    void onPaymentError(const PaymentError& error);
    void onPaymentSuccess();
    void onPaymentToken(const std::string& token);
    void onMobileConnectionChange(const SerialMobileEvent& event);
    void onNFCEvent(const SerialNFCEvent& event, const NFC::DetectionEventData &data);
    void onNFCDetectionError(const NFC::DetectionErrorReason& reason);
    void rejectPendingOperations();
  };
} // namespace JetBeep

#endif