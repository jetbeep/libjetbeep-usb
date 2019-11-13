#include "cmd.h"
#include "../lib/libjetbeep.h"
#include "../lib/device/device_utils.hpp"

#include <stdlib.h>
#include <functional>

using namespace std;
using namespace JetBeep;

Cmd::Cmd()
:m_log("Cmd") {
  m_device.errorCallback = bind(&Cmd::errorHandler, this, placeholders::_1);
  m_device.barcodesCallback = bind(&Cmd::barcodeHandler, this, placeholders::_1);
  m_device.paymentErrorCallback = bind(&Cmd::paymentErrorHandler, this, placeholders::_1);
  m_device.paymentSuccessCallback = bind(&Cmd::paymentSuccessHandler, this);
  m_device.paymentTokenCallback = bind(&Cmd::paymentTokenHandler, this, placeholders::_1);
  m_device.mobileCallback = bind(&Cmd::mobileHandler, this, placeholders::_1);
  m_device.getCallback = bind(&Cmd::getHandler, this, placeholders::_1);
  m_device.getStateCallback = bind(&Cmd::getStateHandler, this, placeholders::_1);
}

void Cmd::process(const string& cmd, const vector<string>& params) {
  if (cmd == "exit") {
    exit(0);
  } else if (cmd == "open") {
    open(params);
  } else if (cmd == "close") {
    close();
  } else if (cmd == "reset_state" || cmd == "resetstate") {
    resetState();
  } else if (cmd == "open_session" || cmd == "opensession") {
    openSession();
  } else if (cmd == "close_session" || cmd == "closesession") {
    closeSession();
  } else if (cmd == "request_barcodes" || cmd == "requestbarcodes") {
    requestBarcodes();
  } else if (cmd == "cancel_barcodes" || cmd == "cancelbarcodes") { 
    cancelBarcodes();
  } else if (cmd == "create_payment" || cmd == "createpayment") {
    createPayment(params);
  } else if (cmd == "cancel_payment" || cmd == "cancelpayment") {
    cancelPayment();
  } else if (cmd == "create_payment_token" || cmd == "createpaymenttoken") {
    createPaymentToken(params);
  } else if (cmd == "get") {
    get(params);
  } else if (cmd == "set") {
    set(params);
  } else if (cmd == "begin_private" || cmd == "beginprivate") {
    beginPrivate();
  } else if (cmd == "commit") {
    commit(params);
  } else if (cmd == "getstate" || cmd == "get_state") {
    getState();
  } else {
    m_log.e() << "invalid command: " << cmd << Logger::endl;
  }
}

void Cmd::open(const vector<string>& params) {
  if (params.size() != 1) {
    m_log.e() << "please select correct device path. E.g. 'open /dev/tty1'" << Logger::endl;
    return;
  }

  auto path = params.at(0);    
  try {
    m_device.open(path);
    m_log.i() << "device opened" << Logger::endl;
  } catch (const exception& e) {
    m_log.e() << "unable to open device: "<< e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to open device" << Logger::endl;
  }
}

void Cmd::close() {
  try {
    m_device.close();
    m_log.i() << "device closed" << Logger::endl;
  } catch (const exception& e) {
    m_log.e() << "unable to close device: "<< e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to close device" << Logger::endl;
  }
}

void Cmd::resetState() {
  try {
    m_device.resetState();
  } catch (const exception& e) {
    m_log.e() << "unable to reset state: "<< e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to reset state" << Logger::endl;
  }   
}

void Cmd::openSession() {
  try {
    m_device.openSession([&](const SerialError& error) {
      if (error != SerialError::noError) {
        m_log.e() << "open session error: " << static_cast<int>(error) << Logger::endl;
      }
    });
    m_log.i() << "session opened" << Logger::endl;
  } catch (const exception& e) {
    m_log.e() << "unable to open session: "<< e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to open session" << Logger::endl;
  }  
}

void Cmd::closeSession() {
  try {
    m_device.closeSession();
    m_log.i() << "session closed" << Logger::endl;
  } catch (const exception& e) {
    m_log.e() << "unable to close session: "<< e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to close session" << Logger::endl;
  } 
}

void Cmd::requestBarcodes() {
  try {
    m_device.requestBarcodes();
    m_log.i() << "barcodes requested" << Logger::endl;
  } catch (const exception& e) {
    m_log.e() << "unable to request barcodes: "<< e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to request barcodes" << Logger::endl;
  }
}

void Cmd::cancelBarcodes() {
  try {
    m_device.cancelBarcodes();
  } catch (const exception& e) {
    m_log.e() << "unable to cancelBarcodes: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to cancelBarcodes" << Logger::endl;
  }  
}

void Cmd::createPayment(const vector<string>& params) {
  auto size = params.size();

  if (size < 2 || size > 4) {
    m_log.e() << "invalid parameters count: " << params.size() << Logger::endl;
    return;
  }  

  auto amount = atoi(params.at(0).c_str());
  auto transactionId = params.at(1);

  try {
    if (size == 2) {
      m_device.createPayment(amount, transactionId);
    } else {
      auto cashierId = params.at(2);
      if (size == 3) {
        m_device.createPayment(amount, transactionId, cashierId);
      } else {
        auto rawMetadata = params.at(3);
        auto splittedMetadata = splitString(rawMetadata, ";");
        PaymentMetadata metadata;

        for (auto it = splittedMetadata.begin(); it != splittedMetadata.end(); ++it) {
          auto keyValue = splitString(*it, ":");

          if (keyValue.size() != 2) {
            throw runtime_error("invalid key value");
          }

          metadata[keyValue.at(0)] = keyValue.at(1);
        }

        m_device.createPayment(amount, transactionId, cashierId, metadata);
      }
    }
  } catch (const exception& e) {
    m_log.e() << "unable to create payment: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to create payment: " << Logger::endl;
  }  
}

void Cmd::cancelPayment() {
  try {
    m_device.cancelPayment();
  } catch (const exception& e) {
    m_log.e() << "unable to cancelPayment: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to cancelPayment" << Logger::endl;
  }  
}

void Cmd::createPaymentToken(const vector<string>& params) {
  auto size = params.size();

  if (size < 2 || size > 4) {
    m_log.e() << "invalid parameters count: " << params.size() << Logger::endl;
    return;
  }  

  auto amount = atoi(params.at(0).c_str());
  auto transactionId = params.at(1);

  try {
    if (size == 2) {
      m_device.createPaymentToken(amount, transactionId);
    } else {
      auto cashierId = params.at(2);
      if (size == 3) {
        m_device.createPaymentToken(amount, transactionId, cashierId);
      } else {
        auto rawMetadata = params.at(3);
        auto splittedMetadata = splitString(rawMetadata, ";");
        PaymentMetadata metadata;

        for (auto it = splittedMetadata.begin(); it != splittedMetadata.end(); ++it) {
          auto keyValue = splitString(*it, ":");

          if (keyValue.size() != 2) {
            throw runtime_error("invalid key value");
          }

          metadata[keyValue.at(0)] = keyValue.at(1);
        }

        m_device.createPaymentToken(amount, transactionId, cashierId, metadata);
      }
    }
  } catch (const exception& e) {
    m_log.e() << "unable to create payment token: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to create payment token: " << Logger::endl;
  }
}

void Cmd::get(const vector<string>& params) {
  if (params.size() != 1) {
    m_log.e() << "invalid get params count: " << params.size() << Logger::endl;
    return;
  }

  try {
    auto parameter = DeviceUtils::stringToParameter(params[0]);
    m_device.get(parameter);
  } catch (const exception& e) {
    m_log.e() << "unable to get: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to get" << Logger::endl;
  }
}

void Cmd::set(const vector<string>& params) {
  if (params.size() != 2) {
    m_log.e() << "invalid set params count: " << params.size() << Logger::endl;
    return;
  }

  try {
    auto parameter = DeviceUtils::stringToParameter(params[0]);
    auto value = params[1];
    m_device.set(parameter, value);
  } catch (const exception& e) {
    m_log.e() << "unable to set: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to set" << Logger::endl;
  }
}

void Cmd::beginPrivate() {
  try {
    m_device.beginPrivate();
  } catch (const exception& e) {
    m_log.e() << "unable to beginPrivate: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to beginPrivate" << Logger::endl;
  }
}

void Cmd::commit(const vector<string>& params) {
  if (params.size() != 1) {
    m_log.e() << "invalid commit params count: " << params.size() << Logger::endl;
    return;
  }

  try {
    auto signature = params[0];
    m_device.commit(signature);
  } catch (const exception& e) {
    m_log.e() << "unable to commit: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to commit" << Logger::endl;
  }
}

void Cmd::getState() {
  try {
    m_device.getState();
  } catch (const exception& e) {
    m_log.e() << "unable to getState: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to getState" << Logger::endl;
  }
}

void Cmd::errorHandler(const SerialError &error) {
	m_log.e() << "device error: " << static_cast<int>(error) << Logger::endl;
}

void Cmd::barcodeHandler(const std::vector<Barcode> &barcodes) {
	m_log.i() << "received " << barcodes.size() << " barcodes" << Logger::endl;
}

void Cmd::paymentErrorHandler(const PaymentError &error) {
	m_log.e() << "payment error: " << static_cast<int>(error) << Logger::endl;
}

void Cmd::paymentSuccessHandler() {
	m_log.i() << "payment successfull" << Logger::endl;
}

void Cmd::paymentTokenHandler(const std::string &token) {
	m_log.i() << "received payment token: " << token << Logger::endl;
}

void Cmd::mobileHandler(const SerialMobileEvent &event) {
  if (event == SerialMobileEvent::connected) {
		m_log.i() << "mobile connected" << Logger::endl;
	} else {
		m_log.i() << "mobile disconnected" << Logger::endl;
	}
}

void Cmd::getHandler(const std::string& result) {
	m_log.i() << "get result: " << result << Logger::endl;
}

void Cmd::getStateHandler(const SerialGetStateResult& result) {
  m_log.i() << "session opened: " << result.isSessionOpened 
    << ", barcodes requested: " << result.isBarcodesRequested 
    << ", payment created: " << result.isPaymentCreated 
    << ", waiting for confirmation: " << result.isWaitingForPaymentConfirmation 
    << ", is refund requested: " << result.isRefundRequested
    << Logger::endl;
}