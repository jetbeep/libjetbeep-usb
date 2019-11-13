#include "serial_device_impl.hpp"
#include "../utils/split.hpp"
#include "device_utils.hpp"

using namespace std;
using namespace JetBeep;
using namespace boost;
using namespace boost::asio;

SerialDevice::Impl::Impl(const SerialDeviceCallbacks& callbacks):
m_thread(&SerialDevice::Impl::runLoop, this), m_work(m_io_service), m_port(m_io_service),
 m_callbacks(callbacks), m_log("device"), m_state(SerialDeviceState::idle), m_timer(m_io_service) {
  
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
  splitted.erase(splitted.begin());

  if (handleResult(command, splitted)) {
    return;
  }

  if (handleResultWithParams(command, splitted)) {
    return;
  }

  if (handleEvent(command, splitted)) {
    return;
  }

  if (*m_callbacks.errorCallback) {
    m_log.e() << "unable to parse command: "<< response << Logger::endl;
    (*m_callbacks.errorCallback)(SerialError::protocolError);
  }
}

bool SerialDevice::Impl::handleResult(const string &command, const vector<string> &params) {
  if (m_state != SerialDeviceState::executeInProgress) {
    return false;
  }

  if (command != m_executedCommand) {
    return false;
  }

  m_state = SerialDeviceState::idle;
  m_timer.cancel();
        
  if (params.size() != 1) {
    m_log.e() << "invalid response params size: " << params.size() << Logger::endl;
    m_executePromise.reject(make_exception_ptr(Errors::InvalidResponse()));
    return true;
  }
  
  auto result = params[0];

  if (result == "ok") {
    m_executePromise.resolve();
  } else {
    m_log.e() << "result is not ok: " << result << Logger::endl;
    m_executePromise.reject(make_exception_ptr(Errors::InvalidResponse()));
  }
  return true;
}

bool SerialDevice::Impl::handleResultWithParams(const std::string &command, const vector<string> &params) {
  if (m_state != SerialDeviceState::executeInProgress) {
    return false;
  }

  if (command != m_executedCommand) {
    return false;
  }

  m_state = SerialDeviceState::idle;
  m_timer.cancel();

  if (command == DeviceResponses::get) {
    if (params.size() != 2) {
      m_log.e() << "invalid get response split size: " << params.size() << Logger::endl;
      m_executeStringPromise.reject(make_exception_ptr(Errors::InvalidResponse()));
      return true;
    }

    auto result = params[0];
    auto value = params[1];

    if (result != "ok") {
      m_log.e() << "result is not ok: " << result << Logger::endl;
      m_executeStringPromise.reject(make_exception_ptr(Errors::InvalidResponse()));
      return true;
    }

    m_executeStringPromise.resolve(value);
    return true;
  } else if (command == DeviceResponses::getState) {
    if (params.size() != 6) {
      m_log.e() << "invalid getState response split size: " << params.size() << Logger::endl;
      m_executeGetStatePromise.reject(make_exception_ptr(Errors::InvalidResponse()));
      return true;
    }
    
    SerialGetStateResult result;
    result.isSessionOpened = params[1] == "1";
    result.isBarcodesRequested = params[2] == "1";
    result.isPaymentCreated = params[3] == "1";
    result.isWaitingForPaymentConfirmation = params[4] == "1";
    result.isRefundRequested = params[5] == "1";

    m_executeGetStatePromise.resolve(result);
    return true;
  }

  return false;
}

bool SerialDevice::Impl::handleEvent(const string& event, const vector<string> &params) {
  if (event == DeviceResponses::mobileConnected) {
    if (*m_callbacks.mobileCallback) {
      (*m_callbacks.mobileCallback)(SerialMobileEvent::connected);
    }
    return true;    
  } else if (event == DeviceResponses::mobileDisconnected) {
    if (*m_callbacks.mobileCallback) {
      (*m_callbacks.mobileCallback)(SerialMobileEvent::disconnected);
    }    
  } else if (event == DeviceResponses::barcodes) {
    vector<Barcode> barcodes;

    if (params.size() % 2 != 0) {
      m_log.e() << "invalid params size when received barcodes: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return true;
    }

    for (auto it = params.begin(), it2 = std::next(it) ; it != params.end(); it += 2) {
        string value = *it;
        int type = atoi((*it2).c_str());
        Barcode barcode = {value, static_cast<BarcodeType>(type)};          

        barcodes.push_back(barcode);
    }

    if (*m_callbacks.barcodesCallback) {
      (*m_callbacks.barcodesCallback)(barcodes);
    }
    return true;
  } else if (event == DeviceResponses::paymentToken) {
    if (params.size() != 1) {
      m_log.e() << "invalid params count of payment token: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return true;
    }

    auto paymentToken = params[0];
    if (*m_callbacks.paymentTokenCallback) {
      (*m_callbacks.paymentTokenCallback)(paymentToken);
    }
  } else if (event == DeviceResponses::paymentError) {
    if (params.size() != 1) {
      m_log.e() << "invalid params count of payment error: " << params.size() << Logger::endl;
      if (*m_callbacks.errorCallback) {
        (*m_callbacks.errorCallback)(SerialError::protocolError);
      }      
      return true;
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
      return true;
    }

    if (*m_callbacks.paymentErrorCallback) {
      (*m_callbacks.paymentErrorCallback)(paymentError);
    }    
  } else if (event == DeviceResponses::paymentSuccessful) {
    if (*m_callbacks.paymentSuccessCallback) {
      (*m_callbacks.paymentSuccessCallback)();
    }
    return true;
  }

  return false;
}

void SerialDevice::Impl::writeSerial(const string& cmd, unsigned int timeoutInMilliseconds) {
  if (m_state != SerialDeviceState::idle) {
    throw Errors::OperationInProgress();
  }

  if (!m_port.is_open()) {
    throw Errors::DeviceNotOpened();
  }

  m_log.d() << "nrf tx: " << cmd.substr(0, cmd.size() - 2) << Logger::endl;

  m_writeData = cmd;
  auto buffer = asio::buffer(m_writeData.c_str(), m_writeData.size());
  auto writeCallback = boost::bind(&SerialDevice::Impl::writeCompleted, this, 
    asio::placeholders::error, asio::placeholders::bytes_transferred);

  async_write(m_port, buffer, writeCallback); 

  // NOTE: expires_from_now cancels all pending timeouts (according to docs)
  m_timer.expires_from_now(boost::posix_time::millisec(timeoutInMilliseconds));
  m_timer.async_wait(boost::bind(&SerialDevice::Impl::handleTimeout, this, asio::placeholders::error));
  
  m_state = SerialDeviceState::executeInProgress;
}

Promise<void> SerialDevice::Impl::execute(const string &cmd, const string& params, unsigned int timeoutInMilliseconds) {
  if (params != "") {
    writeSerial(cmd + " " + params + "\r\n", timeoutInMilliseconds);
  } else {
    writeSerial(cmd + "\r\n", timeoutInMilliseconds);
  }
  m_executePromise = Promise<void>();
  m_executedCommand = cmd;
  return m_executePromise;
}

Promise<string> SerialDevice::Impl::executeString(const string &cmd, const string& params, unsigned int timeoutInMilliseconds) {
  if (params != "") {
    writeSerial(cmd + " " + params + "\r\n", timeoutInMilliseconds);
  } else {
    writeSerial(cmd + "\r\n", timeoutInMilliseconds);
  }
  m_executeStringPromise = Promise<string>();  
  m_executedCommand = cmd;
  return m_executeStringPromise;
}

Promise<SerialGetStateResult> SerialDevice::Impl::executeGetState(const string &cmd, const string& params, unsigned int timeoutInMilliseconds) {
  if (params != "") {
    writeSerial(cmd + " " + params + "\r\n", timeoutInMilliseconds);
  } else {
    writeSerial(cmd + "\r\n", timeoutInMilliseconds);
  }  
  m_executeGetStatePromise = Promise<SerialGetStateResult>();  
  m_executedCommand = cmd;
  return m_executeGetStatePromise;
}

void SerialDevice::Impl::handleTimeout(const boost::system::error_code& err) {
  if (err == boost::asio::error::operation_aborted) {    
    return;
  }

  switch (m_state) {
    case SerialDeviceState::executeInProgress:
      m_state = SerialDeviceState::idle;
      m_executePromise.reject(make_exception_ptr(Errors::OperationTimeout()));      
    break;
    default:
      m_log.e() << "handle timeout call, while no active operation in progress" << Logger::endl;
    break;
  }  
}

void SerialDevice::Impl::runLoop() {
  m_io_service.run();
}