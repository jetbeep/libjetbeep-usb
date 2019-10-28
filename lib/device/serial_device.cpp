#include "serial_device.hpp"
#include "../utils/logger.hpp"
#include "../utils/split.hpp"
#include "device_utils.hpp"

#include <thread>
#include <iterator>

#include <boost/asio/serial_port.hpp> 
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace JetBeep;
using namespace std;
using namespace boost;
using namespace boost::asio;

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

    void open(const string& path);
    void close();

    void execute(const string& cmd);
  private:
    string m_writeData;
    asio::streambuf m_readBuffer;
    Logger m_log;
    SerialDeviceCallbacks m_callbacks;
    io_service m_io_service;
    io_service::work m_work;
    serial_port m_port;
    thread m_thread;    
    void runLoop();

    void writeCompleted(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void readCompleted(const boost::system::error_code& err);
    void handleResponse(const string& response);
    bool handleResult(const string& result, const vector<string>& params);
    void handleEvent(const string& event, const vector<string>& params);
};

SerialDevice::Impl::Impl(const SerialDeviceCallbacks& callbacks):
m_thread(&SerialDevice::Impl::runLoop, this), m_work(m_io_service), m_port(m_io_service), m_callbacks(callbacks), m_log("device") {

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
    m_log.e() << "write error: "<< error << Logger::endl;
    if (*m_callbacks.errorCallback) {
      (*m_callbacks.errorCallback)(SerialError::ioError);
    }    
    return;
  }      
}

void SerialDevice::Impl::readCompleted(const boost::system::error_code& error) {
  if (error) {    
    m_log.e() << "read error: "<< error << Logger::endl;
    if (*m_callbacks.errorCallback) {
      (*m_callbacks.errorCallback)(SerialError::ioError);
    }    
    return;
  }
    
  asio::streambuf::const_buffers_type bufs = m_readBuffer.data();
  string response(asio::buffers_begin(bufs), asio::buffers_begin(bufs) + m_readBuffer.size());

  m_readBuffer.consume(m_readBuffer.size());    
  async_read_until(m_port, m_readBuffer, "\r\n", boost::bind(&SerialDevice::Impl::readCompleted, this, asio::placeholders::error));

  if (response.size() < 2) {    
    m_log.e() << "response size < 2" << Logger::endl;
    if (*m_callbacks.errorCallback) {
      (*m_callbacks.errorCallback)(SerialError::protocolError);
    }    
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
    if (*m_callbacks.errorCallback) {
      (*m_callbacks.errorCallback)(SerialError::protocolError);
    }    
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
  command.compare(DeviceResponses::resetState) == 0 ||
  command.compare(DeviceResponses::beginPrivate) == 0 ||
  command.compare(DeviceResponses::commit) == 0 ||
  command.compare(DeviceResponses::set) == 0
  ) {
    if (splitted.size() != 2) {
      m_log.e() << "invalid response split size: " << splitted.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return;
    }
    auto result = splitted[1];

    if (result == "ok") {
      return;
    } else {
      m_log.e() << "result is not ok: " << result << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return;
    }
  }    
  splitted.erase(splitted.begin());

  if (handleResult(command, splitted)) {
    return;
  }

  handleEvent(command, splitted);  
}

bool SerialDevice::Impl::handleResult(const std::string &result, const vector<string> &params) {
  if (result.compare(DeviceResponses::get) == 0) {
    if (params.size() != 2) {
      m_log.e() << "invalid get response split size: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }
      return true;
    }

    auto result = params[0];
    auto value = params[1];

    if (result != "ok") {
      m_log.e() << "result is not ok: " << result << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return true;
    }
    if (*m_callbacks.getCallback) {
      (*m_callbacks.getCallback)(value);
    }
    return true;
  } else if (result.compare(DeviceResponses::getState) == 0) {
    if (params.size() != 4) {
      m_log.e() << "invalid getState response split size: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }
      return true;
    }
        
    auto isSessionOpened = params[1] == "1";
    auto isBarcodesRequested = params[2] == "1";
    auto isPaymentCreated = params[3] == "1";

    if (*m_callbacks.getStateCallback) {
      (*m_callbacks.getStateCallback)(isSessionOpened, isBarcodesRequested, isPaymentCreated);
    }
  }

  return false;
}

void SerialDevice::Impl::handleEvent(const string& event, const vector<string> &params) {
  if (event.compare(DeviceResponses::mobileConnected) == 0) {
    if (*m_callbacks.mobileCallback) {
      (*m_callbacks.mobileCallback)(SerialMobileEvent::connected);
    }    
  } else if (event.compare(DeviceResponses::mobileDisconnected) == 0) {
    if (*m_callbacks.mobileCallback) {
      (*m_callbacks.mobileCallback)(SerialMobileEvent::disconnected);
    }    
  } else if (event.compare(DeviceResponses::barcodes) == 0) {
    vector<Barcode> barcodes;

    if (params.size() % 2 != 0) {
      m_log.e() << "invalid params size when received barcodes: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return;
    }

    for (auto it = params.begin(), it2 = next(it) ; it != params.end(); it += 2) {
        string value = *it;
        int type = atoi((*it2).c_str());
        Barcode barcode = {value, static_cast<BarcodeType>(type)};          

        barcodes.push_back(barcode);
    }
    if (*m_callbacks.barcodesCallback) {
      (*m_callbacks.barcodesCallback)(barcodes);
    }    
  } else if (event.compare(DeviceResponses::paymentToken) == 0) {
    if (params.size() != 1) {
      m_log.e() << "invalid params count of payment token: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return;
    }

    auto paymentToken = params[0];
    if (*m_callbacks.paymentTokenCallback) {
      (*m_callbacks.paymentTokenCallback)(paymentToken);
    }    
  } else if (event.compare(DeviceResponses::paymentError) == 0) {
    if (params.size() != 1) {
      m_log.e() << "invalid params count of payment error: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return;
    }

    auto paymentErrorStr = params[0];
    PaymentError paymentError = PaymentError::unknown;
    if (paymentErrorStr == "NETWORK") {
      paymentError = PaymentError::network;
    } else if (paymentErrorStr == "TIMEOUT") {
      paymentError = PaymentError::timeout;
    } else if (paymentErrorStr == "SERVER") {
      paymentError = PaymentError::server;
    } else if (paymentErrorStr == "SECURITY") {
      paymentError = PaymentError::security;
    } else if (paymentErrorStr == "WITHDRAWAL") {
      paymentError = PaymentError::withdrawal;
    } else if (paymentErrorStr == "DISCARDED") {
      paymentError = PaymentError::discarded;
    } else if (paymentErrorStr == "INVALID_PIN") {
      paymentError = PaymentError::invalidPin;
    } else if (paymentErrorStr == "UNKNOWN") {
      paymentError = PaymentError::unknown;
    } else {
      m_log.e() << "unable to parse payment error" << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return;
    }

    if (*m_callbacks.paymentErrorCallback) {
      (*m_callbacks.paymentErrorCallback)(paymentError);
    }    
  } else if (event.compare(DeviceResponses::paymentSuccessful)) {
    if (*m_callbacks.paymentSuccessCallback) {
      (*m_callbacks.paymentSuccessCallback)();
    }    
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

SerialDevice::SerialDevice() {
  SerialDeviceCallbacks callbacks = {&errorCallback, &barcodesCallback, &paymentErrorCallback, 
    &paymentSuccessCallback, &paymentTokenCallback, &mobileCallback, &getCallback, &getStateCallback};

  m_impl.reset(new Impl(callbacks));
}

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
void SerialDevice::get(const DeviceParameter& parameter) { m_impl->execute("GET " + DeviceUtils::parameterToString(parameter) + "\r\n"); }
void SerialDevice::set(const DeviceParameter& parameter, const std::string &value) { m_impl->execute("SET " + DeviceUtils::parameterToString(parameter) + " " + value + "\r\n"); }
void SerialDevice::beginPrivate() { m_impl->execute("BEGIN_PRIVATE\r\n"); }
void SerialDevice::commit(const string& signature) { m_impl->execute("COMMIT " + signature + "\r\n"); }
void SerialDevice::getState() { m_impl->execute("GETSTATE \r\n"); }