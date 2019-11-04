#ifndef SERIAL_DEVICE_IMP__H
#define SERIAL_DEVICE_IMP__H

#include "serial_device.hpp"
#include "../utils/logger.hpp"
#include <thread>
#include <iterator>

#include <boost/asio/serial_port.hpp> 
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
    SerialGetCallback *getCallback;
    SerialGetStateCallback *getStateCallback;
  } SerialDeviceCallbacks;

  class SerialDevice::Impl {
  public:
    Impl(const SerialDeviceCallbacks& callbacks);
    virtual ~Impl(); 

    void open(const std::string& path);
    void close();

    std::function<void (const SerialError &)> executeCallback;

    void execute(const std::string& cmd);
  private:
    std::string m_writeData;
    boost::asio::streambuf m_readBuffer;
    Logger m_log;
    SerialDeviceCallbacks m_callbacks;
    boost::asio::io_service m_io_service;
    boost::asio::io_service::work m_work;
    boost::asio::serial_port m_port;
    std::thread m_thread;    
    void runLoop();

    void writeCompleted(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void readCompleted(const boost::system::error_code& err);
    void handleResponse(const std::string& response);
    bool handleResult(const std::string& result, const std::vector<std::string>& params);
    void handleEvent(const std::string& event, const std::vector<std::string>& params);
  };
}

#endif