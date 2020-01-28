#ifndef SERIAL_DEVICE_IMP__H
#define SERIAL_DEVICE_IMP__H

#include "../lib/libjetbeep.hpp"
#include "serial_device.hpp"
#include <iterator>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

namespace DFU {

  enum class SerialDeviceState { idle, executeInProgress };

  class SerialDevice {
  public:
    SerialDevice();
    virtual ~SerialDevice();

    void open(const std::string& path);
    void close();

    uint32_t getDeviceId();
    string getFirmwareVer();

  private:
    string getResponseStr();

    SerialDeviceState m_state;
    std::recursive_mutex m_mutex;
    boost::asio::streambuf m_readBuffer;
    JetBeep::Logger m_log;
    boost::asio::io_service m_io_service;
    boost::asio::serial_port m_port;
    boost::asio::deadline_timer m_timer;
  };
} // namespace JetBeep

#endif