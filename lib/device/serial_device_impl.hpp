#ifndef SERIAL_DEVICE_IMP__H
#define SERIAL_DEVICE_IMP__H

#include "../utils/logger.hpp"
#include "../utils/promise.hpp"
#include "serial_device.hpp"
#include <iterator>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace JetBeep {
  typedef struct SerialDeviceCallbacks {
    SerialErrorCallback* errorCallback;
    SerialBarcodesCallback* barcodesCallback;
    SerialPaymentErrorCallback* paymentErrorCallback;
    SerialPaymentSuccessCallback* paymentSuccessCallback;
    SerialPaymentTokenCallback* paymentTokenCallback;
    SerialMobileCallback* mobileCallback;
  } SerialDeviceCallbacks;

  enum class SerialDeviceState { idle, executeInProgress };

  class SerialDevice::Impl {
  public:
    Impl(const SerialDeviceCallbacks& callbacks, IOContext context);
    virtual ~Impl();

    void open(const std::string& path);
    void close();

    Promise<void> execute(const std::string& cmd, const std::string& params = "", unsigned int timeoutInMilliseconds = 2000);
    Promise<std::string> executeString(const std::string& cmd, const std::string& params = "", unsigned int timeoutInMilliseconds = 2000);
    Promise<SerialGetStateResult> executeGetState(const std::string& cmd, const std::string& params = "", unsigned int timeoutInMilliseconds = 2000);
    void cancelPendingOperations();

  private:
    IOContext m_context;
    SerialDeviceState m_state;
    Promise<void> m_executePromise;
    Promise<std::string> m_executeStringPromise;
    Promise<SerialGetStateResult> m_executeGetStatePromise;
    std::string m_executedCommand;
    std::string m_writeData;
    std::recursive_mutex m_mutex;
    boost::asio::streambuf m_readBuffer;
    Logger m_log;
    SerialDeviceCallbacks m_callbacks;
    boost::asio::serial_port m_port;
    boost::asio::deadline_timer m_timer;
    void writeSerial(const std::string& cmd, unsigned int timeoutInMilliseconds);

    void handleTimeout(const boost::system::error_code& err);
    void writeCompleted(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void readCompleted(const boost::system::error_code& err);
    void handleResponse(const std::string& response);
    bool handleResult(const std::string& command, const std::vector<std::string>& params);
    bool handleResultWithParams(const std::string& command, const std::vector<std::string>& params);
    bool handleEvent(const std::string& event, const std::vector<std::string>& params);
    void rejectPendingPromises(std::exception_ptr exception);
  };
} // namespace JetBeep

#endif