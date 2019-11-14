#ifndef JETBEEP_AUTO_DEVICE_IMPL__H
#define JETBEEP_AUTO_DEVICE_IMPL__H

#include "auto_device.hpp"
#include "serial_device.hpp"
#include "../detection/detection.hpp"
#include "../utils/logger.hpp"
#include "../utils/promise.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <string>

namespace JetBeep {
  class AutoDevice::Impl {
  public:
    Impl(AutoDeviceStateChangeCallback *callback);
    virtual ~Impl();

    void openSession();
    void closeSession();

    Promise<std::vector<Barcode> > requestBarcodes();
    void cancelBarcodes();

    Promise<void> createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", 
      const PaymentMetadata& metadata = PaymentMetadata());    
    void confirmPayment();

    Promise<std::string> createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId = "", 
      const PaymentMetadata& metadata = PaymentMetadata());
    void cancelPayment(); 

    AutoDeviceState state();
  private:
    Promise<std::vector<Barcode> > m_barcodesPromise;
    Promise<void> m_paymentPromise;
    Promise<std::string> m_paymentTokenPromise;
    DeviceCandidate m_candidate;
    Logger m_log;
    AutoDeviceState m_state;
    AutoDeviceStateChangeCallback *m_callback;
    DeviceDetection m_detection;
    SerialDevice m_device;
    boost::asio::io_service m_io_service;
    boost::asio::io_service::work m_work;
    boost::asio::deadline_timer m_timer;
    std::thread m_thread;
    std::recursive_mutex m_mutex;
    std::vector<std::function<void ()> > m_pendingOperations;    
    
    void onDeviceEvent(const DeviceDetectionEvent& event, const DeviceCandidate& candidate);
    void notifyStateChange(AutoDeviceState state, std::exception_ptr exception);
    void resetState();
    void runLoop();
    void handleTimeout(const boost::system::error_code& err);
    void executeNextOperation();
    void enqueueOperation(const std::function<void ()>& callback);
    void onBarcodes(const std::vector<Barcode> &barcodes);
    void onPaymentError(const PaymentError &error);
    void onPaymentSuccess();
    void onPaymentToken(const std::string &token);
  };
}

#endif