#ifndef DFU_SERIAL_DEVICE__HPP
#define DFU_SERIAL_DEVICE__HPP

#include "../lib/libjetbeep.hpp"
#include <iterator>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

namespace DFU {


  class SyncSerialDevice {
  public:
    SyncSerialDevice();
    virtual ~SyncSerialDevice();

    void open(const std::string& path);
    void close();

    uint32_t getDeviceId();
    string getFirmwareVer();
    string getPublicKey();
  
    void enterDFUMode();

    size_t readBytes(void * p_data, size_t size);
    void writeBytes(void * p_data, size_t size);

    bool isBootloaderMode();
  private:
    string getResponseStr();
    string getCmd(string prop);

    boost::asio::streambuf m_readBuffer;
    JetBeep::Logger m_log;
    boost::asio::io_service m_io_service;
    boost::asio::serial_port m_port;
  };
} // namespace JetBeep

#endif