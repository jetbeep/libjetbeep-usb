#include "cmd.hpp"

using namespace std;
using namespace JetBeep;

Cmd::Cmd():m_log("cmd") {

}

void Cmd::process(const std::string& cmd, const std::vector<std::string>& params) {
  if (cmd == "start") {
    start();
  } else if (cmd == "stop") {
    stop();
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
  } else if (cmd == "create_payment_token" || cmd == "createpaymenttoken") {
    createPaymentToken(params);
  } else if (cmd == "confirm_payment" || cmd == "confirmpayment") {
    confirmPayment();
  } else if (cmd == "cancel_payment" || cmd == "cancelpayment") {
    cancelPayment();
  }
}

void Cmd::start() {
  try {
    m_autoDevice.start();
  } catch (...) {
    m_log.e() << "unable to start" << Logger::endl;
  }
}

void Cmd::stop() {
  try {
    m_autoDevice.start();
  } catch (...) {
    m_log.e() << "unable to stop" << Logger::endl;
  }
}

void Cmd::openSession() {
  try {
    m_autoDevice.openSession();
  } catch (...) {
    m_log.e() << "unable to open session" << Logger::endl;
  }  
}

void Cmd::closeSession() {
  try {
    m_autoDevice.closeSession();
  } catch (...) {
    m_log.e() << "unable to close session" << Logger::endl;
  }  
}

void Cmd::requestBarcodes() {
  try {
    m_autoDevice.requestBarcodes()
      .then([&] (const vector<Barcode> &barcodes) {
        m_log.i() << "received "<<barcodes.size() << " barcodes" << Logger::endl;
      }).catchError([&] (exception_ptr error) {
        m_log.e() << "unable to receive barcodes" << Logger::endl;
      });
  } catch (...) {
    m_log.e() << "unable to request barcodes" << Logger::endl;
  }
}

void Cmd::cancelBarcodes() {
  try {
    m_autoDevice.cancelBarcodes();
  } catch (...) {
    m_log.e() << "unable to cancel barcodes" << Logger::endl;
  }
}

void Cmd::createPayment(const vector<string> &params) {
  auto size = params.size();

  if (size < 2 || size > 4) {
    m_log.e() << "invalid parameters count: " << params.size() << Logger::endl;
    return;
  }  

  auto amount = atoi(params.at(0).c_str());
  auto transactionId = params.at(1);
  auto cashierId = "";
  auto metadata = PaymentMetadata();

  try {
    if (size > 2) {
      auto cashierId = params.at(2);
      if (size == 4) {
        auto rawMetadata = params.at(3);
        auto splittedMetadata = Utils::splitString(rawMetadata, ";");

        for (auto it = splittedMetadata.begin(); it != splittedMetadata.end(); ++it) {
          auto keyValue = Utils::splitString(*it, ":");

          if (keyValue.size() != 2) {
            throw runtime_error("invalid key value");
          }

          metadata[keyValue.at(0)] = keyValue.at(1);
        } 
      }
    }
  } catch (...) {
    m_log.e() << "unable to create payment" << Logger::endl;
  }
  
  m_autoDevice.createPayment(amount, transactionId, cashierId, metadata)
    .then([&] {
      m_log.i() << "payment successful" << Logger::endl;
    }).catchError([&] (exception_ptr error) {
      m_log.e() << "payment is not successful" << Logger::endl;
    });    
}

void Cmd::createPaymentToken(const vector<string> &params) {
  auto size = params.size();

  if (size < 2 || size > 4) {
    m_log.e() << "invalid parameters count: " << params.size() << Logger::endl;
    return;
  }  

  auto amount = atoi(params.at(0).c_str());
  auto transactionId = params.at(1);
  auto cashierId = "";
  auto metadata = PaymentMetadata();

  try {
    if (size > 2) {
      auto cashierId = params.at(2);
      if (size == 4) {
        auto rawMetadata = params.at(3);
        auto splittedMetadata = Utils::splitString(rawMetadata, ";");

        for (auto it = splittedMetadata.begin(); it != splittedMetadata.end(); ++it) {
          auto keyValue = Utils::splitString(*it, ":");

          if (keyValue.size() != 2) {
            throw runtime_error("invalid key value");
          }

          metadata[keyValue.at(0)] = keyValue.at(1);
        } 
      }
    }
  } catch (...) {
    m_log.e() << "unable to create payment" << Logger::endl;
  }
  
  m_autoDevice.createPaymentToken(amount, transactionId, cashierId, metadata)
    .then([&] (string token) {
      m_log.i() << "token: " << token << Logger::endl;
    }).catchError([&] (exception_ptr error) {
      m_log.e() << "unable to create payment token" << Logger::endl;
    });    
}

void Cmd::confirmPayment() {
  try {
    m_autoDevice.confirmPayment();
  } catch (...) {
    m_log.e() << "unable to confirm payment" << Logger::endl;
  }  
}

void Cmd::cancelPayment() {
  try {
    m_autoDevice.cancelPayment();
  } catch (...) {
    m_log.e() << "unable to cancel payment" << Logger::endl;
  }    
}