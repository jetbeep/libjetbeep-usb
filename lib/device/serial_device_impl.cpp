#include "serial_device_impl.hpp"
#include "../utils/split.hpp"
#include "device_utils.hpp"

using namespace std;
using namespace JetBeep;
using namespace boost;
using namespace boost::asio;

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
    if (params.size() != 6) {
      m_log.e() << "invalid getState response split size: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }
      return true;
    }
    
    SerialGetStateResult result;
    result.isSessionOpened = params[1] == "1";
    result.isBarcodesRequested = params[2] == "1";
    result.isPaymentCreated = params[3] == "1";
    result.isWaitingForPaymentConfirmation = params[4] == "1";
    result.isRefundRequested = params[5] == "1";

    if (*m_callbacks.getStateCallback) {
      (*m_callbacks.getStateCallback)(result);
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
  } else if (event.compare(DeviceResponses::paymentSuccessful) == 0) {
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
  m_io_service.run();
}