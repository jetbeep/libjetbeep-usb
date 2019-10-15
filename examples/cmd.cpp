#include "cmd.h"
#include "../lib/libjetbeep.h"

#include <stdlib.h>

using namespace std;
using namespace JetBeep;

Cmd::Cmd()
:m_log("Cmd") {

}

void Cmd::process(const string& cmd, const vector<string>& params) {
  if (cmd == "exit") {
    exit(0);
  } else if (cmd == "open") {
    open(params);
  } else if (cmd == "close") {
    close();
  } else if (cmd == "open_session" || cmd == "opensession") {
    openSession();
  } else if (cmd == "close_session" || cmd == "closesession") {
    closeSession();
  } else if (cmd == "request_barcodes" || cmd == "requestbarcodes") {
    requestBarcodes();
  } else if (cmd == "create_payment" || cmd == "createpayment") {
    createPayment(params);
  } else if (cmd == "create_payment_token" || cmd == "createpaymenttoken") {
    createPaymentToken(params);
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

void Cmd::openSession() {
  try {
    m_device.openSession();
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