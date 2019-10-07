#include "device.hpp"
#include "../utils/logger.hpp"
#include "../utils/split.hpp"

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

    vector<Barcode> barcodes;
    int errorCode;
    string paymentToken;
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
    void handleResponse(const string& response);
    void notify(const DeviceEvent& event);
};

namespace DeviceResponses {
  // responses
  const string openSession = "OPEN_SESSION";
  const string closeSession = "CLOSE_SESSION";
  const string requestBarcodes = "REQUEST_BARCODES";
  const string createPayment = "CREATE_PAYMENT";
  const string cancelPayment = "CANCEL_PAYMENT";
  const string confirmPayment = "CONFIRM_PAYMENT";
  const string createPaymentToken = "CREATE_PAYMENT_TOKEN";

  // events
  const string mobileConnected = "MOBILE_CONNECTED";
  const string mobileDisconnected = "MOBILE_DISCONNECTED";
  const string barcodes = "BARCODES";
  const string paymentSuccessful = "PAYMENT_SUCCESSFUL";
  const string paymentError = "PAYMENT_ERROR";
  const string paymentToken = "PAYMENT_TOKEN";

  DeviceEvent toDeviceEvent(const string &event) {
    if (event.compare(DeviceResponses::mobileConnected) == 0) {
      return DeviceEvent::mobileConnected;
    } else if (event.compare(DeviceResponses::mobileDisconnected) == 0) {
      return DeviceEvent::mobileDisconnected;
    } else if (event.compare(DeviceResponses::barcodes) == 0) {
      return DeviceEvent::barcodes;
    } else if (event.compare(DeviceResponses::paymentSuccessful) == 0) {
      return DeviceEvent::paymentSuccessful;
    } else if (event.compare(DeviceResponses::paymentError) == 0) {
      return DeviceEvent::paymentError;
    } else if (event.compare(DeviceResponses::paymentToken) == 0) {
      return DeviceEvent::paymentToken;
    } else {
      throw runtime_error("invalid event string");
    }
  }
}

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
  if (error) {
    errorCode = error.value();
    notify(DeviceEvent::deviceError);
    m_log.e() << "write error: "<< error << Logger::endl;
    return;
  }      
}

void Device::Impl::readCompleted(const boost::system::error_code& error) {
  if (error) {          
    errorCode = error.value();
    notify(DeviceEvent::deviceError);
    m_log.e() << "read error: "<< error << Logger::endl;
    return;
  }
  
  asio::streambuf::const_buffers_type bufs = m_readBuffer.data();
  string response(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + m_readBuffer.size());

  if (response.size() < 2) {
    errorCode = -1;
    notify(DeviceEvent::deviceError);
    m_log.e() << "response size < 2" << Logger::endl;
    return;
  }

  response = response.substr(0, response.size() - 2);
  handleResponse(response);
}

void Device::Impl::handleResponse(const string &response) {
  auto splitted = splitString(response);

  m_log.d() << "nrf rx: " << response << Logger::endl;

  if (splitted.empty()) {
    m_log.e() << "unable to split string..."<< Logger::endl;
    errorCode = -2;
    notify(DeviceEvent::protocolError);
    return;
  }

  auto command = splitted.at(0);

  if (command.compare(DeviceResponses::openSession) == 0 ||
  command.compare(DeviceResponses::closeSession) == 0 ||
  command.compare(DeviceResponses::requestBarcodes) == 0 ||
  command.compare(DeviceResponses::createPayment) == 0 ||
  command.compare(DeviceResponses::cancelPayment) == 0 ||
  command.compare(DeviceResponses::confirmPayment) == 0 || 
  command.compare(DeviceResponses::createPaymentToken) == 0
  ) {
    if (splitted.size() != 2) {
      m_log.e() << "invalid response split size: " << splitted.size() << Logger::endl;
      errorCode = -3;
      notify(DeviceEvent::protocolError);
      return;
    }
    auto result = splitted.at(1);

    if (result == "ok") {
      return;
    } else {
      errorCode = -1;
      notify(DeviceEvent::protocolError);
      return;
    }
  }

  DeviceEvent event;
  try {
    event = DeviceResponses::toDeviceEvent(response);
  } catch (...) {
    errorCode = -2;
    notify(DeviceEvent::protocolError);
    return;
  }

  switch (event) {
    case DeviceEvent::barcodes:
      barcodes.clear();

      if (splitted.size() % 2 != 1) {
        m_log.e() << "invalid response split size: " << splitted.size() << Logger::endl;
        errorCode = -4;
        notify(DeviceEvent::protocolError);
        return;
      }
      
      for (auto it = splitted.begin()++ ; it != splitted.end(); it += 2) {
          auto it2 = it++;
          string value = *it;
          int type = atoi((*it2).c_str());
          Barcode barcode = {value, type};

          barcodes.push_back(barcode);
      }
      break;
    case DeviceEvent::paymentToken:
      if (splitted.size() != 2) {
        m_log.e() << "invalid response split size: " << splitted.size() << Logger::endl;
        errorCode = -4;
        notify(DeviceEvent::protocolError);
        return;
      }

      paymentToken = splitted.at(1);
      break;
    default:
      break;
  } 

  notify(event);
}

void Device::Impl::notify(const DeviceEvent& event) {
  auto callback = *m_callback;

  if (callback != nullptr) {
    callback(event);
  }
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
const vector<Barcode>& Device::barcodes() { return m_impl->barcodes; }
int Device::errorCode() { return m_impl->errorCode; }
const string& Device::paymentToken() { return m_impl->paymentToken; }