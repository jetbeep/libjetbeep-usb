#include "serial_device.hpp"
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

class SerialDevice::Impl {
  public:
    Impl(DeviceCallback *callback);
    virtual ~Impl(); 

    void open(const string& path);
    void close();

    void execute(const string& cmd);

    vector<Barcode> barcodes;
    int errorCode;
    string paymentToken;
    string paymentError;
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
  const string cancelBarcodes = "CANCEL_BARCODES";
  const string createPayment = "CREATE_PAYMENT";
  const string cancelPayment = "CANCEL_PAYMENT";
  const string confirmPayment = "CONFIRM_PAYMENT";
  const string createPaymentToken = "CREATE_PAYMENT_TOKEN";
  const string resetState = "RESET_STATE";

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

SerialDevice::Impl::Impl(DeviceCallback *callback):
m_thread(&SerialDevice::Impl::runLoop, this), m_work(m_io_service), m_port(m_io_service), m_callback(callback), m_log("device") {

}

SerialDevice::Impl::~Impl() {
  m_io_service.stop();
  m_thread.join();
}

void SerialDevice::Impl::open(const string& path) {
  m_port.open(path);
  m_port.set_option(serial_port_base::baud_rate(9600));

  async_read_until(m_port, m_readBuffer, "\r\n", boost::bind(&SerialDevice::Impl::readCompleted, this, asio::placeholders::error));
}

void SerialDevice::Impl::close() {
  m_port.close();
}

void SerialDevice::Impl::writeCompleted(const boost::system::error_code& error, std::size_t bytes_transferred) {
  if (error) {
    errorCode = error.value();
    notify(DeviceEvent::deviceError);
    m_log.e() << "write error: "<< error << Logger::endl;
    return;
  }      
}

void SerialDevice::Impl::readCompleted(const boost::system::error_code& error) {
  if (error) {          
    errorCode = error.value();
    notify(DeviceEvent::deviceError);
    m_log.e() << "read error: "<< error << Logger::endl;
    return;
  }
    
  asio::streambuf::const_buffers_type bufs = m_readBuffer.data();
  string response(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + m_readBuffer.size());

  m_readBuffer.consume(m_readBuffer.size());    
  async_read_until(m_port, m_readBuffer, "\r\n", boost::bind(&SerialDevice::Impl::readCompleted, this, asio::placeholders::error));

  if (response.size() < 2) {
    errorCode = -1;
    notify(DeviceEvent::deviceError);
    m_log.e() << "response size < 2" << Logger::endl;
    return;
  }

  response = response.substr(0, response.size() - 2);
  handleResponse(response);
}

void SerialDevice::Impl::handleResponse(const string &response) {
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
  command.compare(DeviceResponses::cancelBarcodes) == 0 ||
  command.compare(DeviceResponses::createPayment) == 0 ||
  command.compare(DeviceResponses::cancelPayment) == 0 ||
  command.compare(DeviceResponses::confirmPayment) == 0 || 
  command.compare(DeviceResponses::createPaymentToken) == 0 ||
  command.compare(DeviceResponses::resetState) == 0
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
          Barcode barcode = {value, (BarcodeType)type};          

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
    case DeviceEvent::paymentError:
      if (splitted.size() != 2) {
        m_log.e() << "invalid response split size: " << splitted.size() << Logger::endl;
        errorCode = -4;
        notify(DeviceEvent::protocolError);
        return;
      }

      paymentError = splitted.at(1);
      break;
    default:
      break;
  } 

  notify(event);
}

void SerialDevice::Impl::notify(const DeviceEvent& event) {
  auto callback = *m_callback;

  if (callback != nullptr) {
    callback(event);
  }
}

void SerialDevice::Impl::execute(const string &cmd) {
  if (!m_port.is_open()) {
    throw runtime_error("port is not opened");
  }

  m_log.d() << "nrf tx: " << cmd.substr(0, cmd.size() - 2) << Logger::endl;

  m_writeData = cmd;
  auto buffer = asio::buffer(m_writeData.c_str(), m_writeData.size());
  auto writeCallback = boost::bind(&SerialDevice::Impl::writeCompleted, this, 
    asio::placeholders::error, asio::placeholders::bytes_transferred);

  async_write(m_port, buffer, writeCallback); 
}

void SerialDevice::Impl::runLoop() {
  m_log.i() << "run loop" << Logger::endl;
  m_io_service.run();
  m_log.i() << "run loop end" << Logger::endl;
}

// Device

SerialDevice::SerialDevice(DeviceCallback callback)
:callback(callback), m_impl(new Impl(&this->callback)) {}

SerialDevice::~SerialDevice() {}

void SerialDevice::open(const string& path) { m_impl->open(path); }
void SerialDevice::close() { m_impl->close(); }
void SerialDevice::openSession() { m_impl->execute("OPEN_SESSION\r\n"); }
void SerialDevice::closeSession() { m_impl->execute("CLOSE_SESSION\r\n"); }
void SerialDevice::requestBarcodes() { m_impl->execute("REQUEST_BARCODES\r\n"); }
void SerialDevice::cancelBarcodes() { m_impl->execute("CANCEL_BARCODES\r\n"); }

void SerialDevice::createPayment(uint32_t amount, const std::string& transactionId, const std::string& cashierId, 
  const PaymentMetadata& metadata) {
    ostringstream ss;

    ss << "CREATE_PAYMENT " << amount << " " << transactionId;

    if (cashierId != "") {
      ss << " " << cashierId;

      if (!metadata.empty()) {
        ss << " ";

        for (auto it = metadata.begin(); it != metadata.end(); ++it) {
          ss << (*it).first << ":" << (*it).second;           
        }

        ss << ";";  
      }
    }
    ss << "\r\n";    
    m_impl->execute(ss.str());
}

void SerialDevice::createPaymentToken(uint32_t amount, const std::string& transactionId, const std::string& cashierId, 
  const PaymentMetadata &metadata) {
    ostringstream ss;

    ss << "CREATE_PAYMENT_TOKEN " << amount << " " << transactionId;

    if (cashierId != "") {
      ss << " " << cashierId;

      if (!metadata.empty()) {
        ss << " ";

        for (auto it = metadata.begin(); it != metadata.end(); ++it) {
          ss << (*it).first << ":" << (*it).second;           
        }

        ss << ";";    
      }
    }
    ss << "\r\n";    
    m_impl->execute(ss.str());
}

void SerialDevice::cancelPayment() { m_impl->execute("CANCEL_PAYMENT\r\n"); }
void SerialDevice::resetState() { m_impl->execute("RESET_STATE\r\n"); }
const vector<Barcode>& SerialDevice::barcodes() { return m_impl->barcodes; }
int SerialDevice::errorCode() { return m_impl->errorCode; }
const string& SerialDevice::paymentToken() { return m_impl->paymentToken; }
const string& SerialDevice::paymentError() { return m_impl->paymentError; }