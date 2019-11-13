#ifndef SERIAL_DEVICE_IMP__H
#define SERIAL_DEVICE_IMP__H

#include "serial_device.hpp"
#include "../utils/logger.hpp"
#include "../utils/promise.hpp"
#include <thread>
#include <iterator>

#include <boost/asio/serial_port.hpp> 
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace JetBeep {
  typedef struct SerialDeviceCallbacks {
    SerialErrorCallback *errorCallback;
    SerialBarcodesCallback *barcodesCallback;
    SerialPaymentErrorCallback *paymentErrorCallback;
    SerialPaymentSuccessCallback *paymentSuccessCallback;
    SerialPaymentTokenCallback *paymentTokenCallback;
    SerialMobileCallback *mobileCallback;
  } SerialDeviceCallbacks;

  enum class SerialDeviceState {
    idle,
    executeInProgress
  };

  class SerialDevice::Impl {
  public:
    Impl(const SerialDeviceCallbacks& callbacks);
    virtual ~Impl();

    void open(const std::string& path);
    void close();
    
    Promise<void> execute(const std::string& cmd, const std::string& params = "", unsigned int timeoutInMilliseconds = 2000);
    Promise<std::string> executeString(const std::string &cmd, const std::string& params = "", unsigned int timeoutInMilliseconds = 2000);
    Promise<SerialGetStateResult> executeGetState(const std::string &cmd, const std::string& params = "", unsigned int timeoutInMilliseconds = 2000);
    void cancelPendingOperations();
  private:
    SerialDeviceState m_state;
    Promise<void> m_executePromise;
    Promise<std::string> m_executeStringPromise;
    Promise<SerialGetStateResult> m_executeGetStatePromise;
    std::string m_executedCommand;
    std::string m_writeData;
    boost::asio::streambuf m_readBuffer;
    Logger m_log;
    SerialDeviceCallbacks m_callbacks;
    boost::asio::io_service m_io_service;
    boost::asio::io_service::work m_work;
    boost::asio::serial_port m_port;
    boost::asio::deadline_timer m_timer;
    std::thread m_thread;    
    void runLoop();
    void writeSerial(const std::string &cmd, unsigned int timeoutInMilliseconds);

    void handleTimeout(const boost::system::error_code& err);
    void writeCompleted(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void readCompleted(const boost::system::error_code& err);
    void handleResponse(const std::string& response);
    bool handleResult(const std::string& command, const std::vector<std::string>& params);
    bool handleResultWithParams(const std::string& command, const std::vector<std::string>& params);
    bool handleEvent(const std::string& event, const std::vector<std::string>& params);
  };
}

#endif