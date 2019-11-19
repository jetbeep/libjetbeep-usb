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
    Impl(AutoDeviceStateCallback* stateCallback, AutoDevicePaymentErrorCallback* paymentErrorCallback, SerialMobileCallback* mobileCallback, IOContext context);
    virtual ~Impl();

    void start();
    void stop();

    void openSession();
    void closeSession();

    Promise<std::vector<Barcode>> requestBarcodes();
    void cancelBarcodes();

    Promise<void> createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", const PaymentMetadata& metadata = PaymentMetadata());
    void confirmPayment();

    Promise<std::string> createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", const PaymentMetadata& metadata = PaymentMetadata());
    void cancelPayment();

    AutoDeviceState state();
    bool isMobileConnected();

  private:
    IOContext m_context;
    bool m_mobileConnected;
    AutoDeviceStateCallback* m_stateCallback;
    AutoDevicePaymentErrorCallback* m_paymentErrorCallback;
    SerialMobileCallback* m_mobileCallback;
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

    void onDeviceEvent(const DeviceDetectionEvent& event, const DeviceCandidate& candidate);
    void changeState(AutoDeviceState state, std::exception_ptr exception = nullptr);
    void resetState();
    void handleTimeout(const boost::system::error_code& err);
    void executeNextOperation();
    void enqueueOperation(const std::function<void()>& callback);
    void onBarcodes(const std::vector<Barcode>& barcodes);
    void onPaymentError(const PaymentError& error);
    void onPaymentSuccess();
    void onPaymentToken(const std::string& token);
    void onMobileConnectionChange(const SerialMobileEvent& event);
    void rejectPendingOperations();
  };
} // namespace JetBeep

#endif