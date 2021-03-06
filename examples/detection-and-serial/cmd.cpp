#include "cmd.hpp"
#include "../../lib/device/device_utils.hpp"
#include "../../lib/libjetbeep.hpp"

#include <functional>
#include <stdlib.h>

using namespace std;
using namespace JetBeep;

Cmd::Cmd() : m_log("Cmd") {
  m_device.errorCallback = bind(&Cmd::errorHandler, this, placeholders::_1);
  m_device.barcodesCallback = bind(&Cmd::barcodeHandler, this, placeholders::_1);
  m_device.paymentErrorCallback = bind(&Cmd::paymentErrorHandler, this, placeholders::_1);
  m_device.paymentSuccessCallback = bind(&Cmd::paymentSuccessHandler, this);
  m_device.paymentTokenCallback = bind(&Cmd::paymentTokenHandler, this, placeholders::_1);
  m_device.mobileCallback = bind(&Cmd::mobileHandler, this, placeholders::_1);
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
    beginPrivate(params);
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
    m_log.e() << "unable to open device: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to open device" << Logger::endl;
  }
}

void Cmd::close() {
  try {
    m_device.close();
    m_log.i() << "device closed" << Logger::endl;
  } catch (const exception& e) {
    m_log.e() << "unable to close device: " << e.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unable to close device" << Logger::endl;
  }
}

void Cmd::resetState() {
  try {
    m_device.resetState().then([&] { m_log.i() << "state reset" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::openSession() {
  try {
    m_device.openSession().then([&] { m_log.i() << "session opened" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::closeSession() {
  try {
    m_device.closeSession().then([&] { m_log.i() << "session closed" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::requestBarcodes() {
  try {
    m_device.requestBarcodes().then([&] { m_log.i() << "barcodes ok" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::cancelBarcodes() {
  try {
    m_device.cancelBarcodes().then([&] { m_log.i() << "barcodes cancelled" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
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
      m_device.createPayment(amount, transactionId).then([&] { m_log.i() << "payment created" << Logger::endl; }).catchError([&](exception_ptr error) {
        processError(error);
      });
    } else {
      auto cashierId = params.at(2);
      if (size == 3) {
        m_device.createPayment(amount, transactionId, cashierId)
          .then([&] { m_log.i() << "payment created" << Logger::endl; })
          .catchError([&](exception_ptr error) { processError(error); });
      } else {
        auto rawMetadata = params.at(3);
        auto splittedMetadata = Utils::splitString(rawMetadata, ";");
        PaymentMetadata metadata;

        for (auto it = splittedMetadata.begin(); it != splittedMetadata.end(); ++it) {
          auto keyValue = Utils::splitString(*it, ":");

          if (keyValue.size() != 2) {
            throw runtime_error("invalid key value");
          }

          metadata[keyValue.at(0)] = keyValue.at(1);
        }

        m_device.createPayment(amount, transactionId, cashierId, metadata)
          .then([&] { m_log.i() << "payment created" << Logger::endl; })
          .catchError([&](exception_ptr error) { processError(error); });
      }
    }
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::cancelPayment() {
  try {
    m_device.cancelPayment().then([&] { m_log.i() << "payment cancelled" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
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
      m_device.createPaymentToken(amount, transactionId)
        .then([&] { m_log.i() << "payment token created" << Logger::endl; })
        .catchError([&](exception_ptr error) { processError(error); });
    } else {
      auto cashierId = params.at(2);
      if (size == 3) {
        m_device.createPaymentToken(amount, transactionId, cashierId)
          .then([&] { m_log.i() << "payment token created" << Logger::endl; })
          .catchError([&](exception_ptr error) { processError(error); });
      } else {
        auto rawMetadata = params.at(3);
        auto splittedMetadata = Utils::splitString(rawMetadata, ";");
        PaymentMetadata metadata;

        for (auto it = splittedMetadata.begin(); it != splittedMetadata.end(); ++it) {
          auto keyValue = Utils::splitString(*it, ":");

          if (keyValue.size() != 2) {
            throw runtime_error("invalid key value");
          }

          metadata[keyValue.at(0)] = keyValue.at(1);
        }

        m_device.createPaymentToken(amount, transactionId, cashierId, metadata)
          .then([&] { m_log.i() << "payment token created" << Logger::endl; })
          .catchError([&](exception_ptr error) { processError(error); });
      }
    }
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::get(const vector<string>& params) {
  if (params.size() != 1) {
    m_log.e() << "invalid get params count: " << params.size() << Logger::endl;
    return;
  }

  try {
    auto parameter = DeviceUtils::stringToParameter(params[0]);
    m_device.get(parameter).then([&](string result) { m_log.i() << "get: " << result << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
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
    m_device.set(parameter, value).then([&] { m_log.i() << "set ok" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::beginPrivate(const vector<string>& params) {
  SerialBeginPrivateMode mode;

  if (params.size() != 1) {
    m_log.e() << "invalid begin private params count: " << params.size() << Logger::endl;
    return;
  }

  auto modeString = Utils::toLowerCase(params[0]);
  if (modeString == "setup") {
    mode = SerialBeginPrivateMode::setup;
  } else if (modeString == "config") {
    mode = SerialBeginPrivateMode::config;
  } else {
    m_log.e() << "invalid mode parameter: " << modeString << Logger::endl;
  }

  try {
    m_device.beginPrivate(mode).then([&] { m_log.i() << "begin private ok" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::commit(const vector<string>& params) {
  if (params.size() != 1) {
    m_log.e() << "invalid commit params count: " << params.size() << Logger::endl;
    return;
  }

  try {
    auto signature = params[0];
    m_device.commit(signature).then([&] { m_log.i() << "commit ok" << Logger::endl; }).catchError([&](exception_ptr error) {
      processError(error);
    });
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::getState() {
  try {
    m_device.getState()
      .then([&](SerialGetStateResult result) {
        m_log.i() << "isSessionOpened: " << result.isSessionOpened << " isBarcodesRequested: " << result.isBarcodesRequested
                  << " isPaymentCreated: " << result.isPaymentCreated
                  << " isWaitingForPaymentConfirmation: " << result.isWaitingForPaymentConfirmation
                  << " isRefundRequested: " << result.isRefundRequested << Logger::endl;
      })
      .catchError([&](exception_ptr error) { processError(error); });
  } catch (...) {
    processError(std::current_exception());
  }
}

void Cmd::errorHandler(exception_ptr exception) {
  try {
    rethrow_exception(exception);
  } catch (const Errors::IOError& error) {
    m_log.e() << "device error: " << error.what() << Logger::endl;
  } catch (const Errors::ProtocolError& error) {
    m_log.e() << "device error: " << error.what() << Logger::endl;
  }
}

void Cmd::barcodeHandler(const std::vector<Barcode>& barcodes) {
  m_log.i() << "received " << barcodes.size() << " barcodes" << Logger::endl;
}

void Cmd::paymentErrorHandler(const PaymentError& error) {
  m_log.e() << "payment error: " << static_cast<int>(error) << Logger::endl;
}

void Cmd::paymentSuccessHandler() {
  m_log.i() << "payment successfull" << Logger::endl;
}

void Cmd::paymentTokenHandler(const std::string& token) {
  m_log.i() << "received payment token: " << token << Logger::endl;
}

void Cmd::mobileHandler(const SerialMobileEvent& event) {
  if (event == SerialMobileEvent::connected) {
    m_log.i() << "mobile connected" << Logger::endl;
  } else {
    m_log.i() << "mobile disconnected" << Logger::endl;
  }
}

void Cmd::processError(std::exception_ptr error) {
  try {
    rethrow_exception(error);
  } catch (const Errors::OperationTimeout& error) {
    m_log.e() << error.what() << Logger::endl;
  } catch (const Errors::DeviceNotOpened& error) {
    m_log.e() << error.what() << Logger::endl;
  } catch (const Errors::OperationInProgress& error) {
    m_log.e() << error.what() << Logger::endl;
  } catch (const Errors::InvalidResponse& error) {
    m_log.e() << error.what() << Logger::endl;
  } catch (...) {
    m_log.e() << "unknown exception" << Logger::endl;
  }
}