#include "device.hpp"
#include "../utils/logger.hpp"

#include <thread>

#include <boost/asio/serial_port.hpp> 
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace JetBeep;
using namespace std;
using namespace boost;
using namespace boost::asio;

class Device::Impl {
  public:
    Impl(DeviceCallback *callback);
    virtual ~Impl(); 

    void open(const string& path);
    void close();

    void openSession();
  private:
    string m_writeData;
    asio::streambuf m_readBuffer;
    Logger m_log;
    DeviceCallback *m_callback;
    io_service m_io_service;
    io_service::work m_work;
    serial_port m_port;
    thread m_thread;
    void runLoop();

    void writeCompleted(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void readCompleted(const boost::system::error_code& err);
};

Device::Impl::Impl(DeviceCallback *callback):
m_thread(&Device::Impl::runLoop, this), m_work(m_io_service), m_port(m_io_service), m_callback(callback), m_log("device") {

}

Device::Impl::~Impl() {
  m_io_service.stop();
  m_thread.join();
}

void Device::Impl::open(const string& path) {
  m_port.open(path);
  m_port.set_option(serial_port_base::baud_rate(9600));

  async_read_until(m_port, m_readBuffer, "\r\n", boost::bind(&Device::Impl::readCompleted, this, asio::placeholders::error));
}

void Device::Impl::close() {
  m_port.close();
}

void Device::Impl::writeCompleted(const boost::system::error_code& error, std::size_t bytes_transferred) {
  auto callback = *this->m_callback;

  if (error) {
    m_log.e() << "write error: "<< error << Logger::endl;
    if (callback != nullptr) {
      DeviceEventParameters param;      
      param.errorCode = error.value();      
      callback(WRITE_ERROR, param);
    }
    return;
  }      
}

void Device::Impl::readCompleted(const boost::system::error_code& error) {
  auto callback = *this->m_callback;

  if (error) {
    m_log.e() << "read error: "<< error << Logger::endl;
    if (callback != nullptr) {
      DeviceEventParameters param;      
      param.errorCode = error.value();      
      callback(WRITE_ERROR, param);
    }
    return;
  }
  
  asio::streambuf::const_buffers_type bufs = m_readBuffer.data();
  string response(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + m_readBuffer.size());

  m_log.d() << "read: " << response << Logger::endl;
}

void Device::Impl::openSession() {
  m_writeData = "OPEN_SESSION\r\n";
  auto buffer = asio::buffer(m_writeData.c_str(), m_writeData.size());
  auto writeCallback = boost::bind(&Device::Impl::writeCompleted, this, 
    asio::placeholders::error, asio::placeholders::bytes_transferred);

  async_write(m_port, buffer, writeCallback); 
}

void Device::Impl::runLoop() {
  m_log.i() << "run loop" << Logger::endl;
  m_io_service.run();
  m_log.i() << "run loop end" << Logger::endl;
}

// Device

Device::Device(DeviceCallback callback)
:callback(callback), m_impl(new Impl(&this->callback)) {}

Device::~Device() {}

void Device::open(const string& path) { m_impl->open(path); }
void Device::close() { m_impl->close(); }
void Device::openSession() { m_impl->openSession(); }