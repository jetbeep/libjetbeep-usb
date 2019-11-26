#include "../../lib/libjetbeep.hpp"

#include <random>
#include <string>

using namespace JetBeep;
using namespace std;

#define DEVICE_ID   44
#define CASHIER_ID "test_cashier_1"

#define MERCHANT_SECRET_KEY "F09612A780C041D3939EE8C9CE8DC560"

Logger l("main");

void onStateChange(AutoDeviceState state, std::exception_ptr error) {
  switch (state) {
  case AutoDeviceState::firmwareVersionNotSupported:
    l.i() << "changed state to: firmwareVersionNotSupported" << Logger::endl;
    break;
  case AutoDeviceState::invalid:
    l.i() << "changed state to: invalid" << Logger::endl;
    break;
  case AutoDeviceState::sessionClosed:
    l.i() << "changed state to: sessionClosed" << Logger::endl;
    break;
  case AutoDeviceState::sessionOpened:
    l.i() << "changed state to: sessionOpened" << Logger::endl;
    break;
  case AutoDeviceState::waitingForBarcodes:
    l.i() << "changed state to: waitingForBarcodes" << Logger::endl;
    break;
  case AutoDeviceState::waitingForConfirmation:
    l.i() << "changed state to: waitingForConfirmation" << Logger::endl;
    break;
  case AutoDeviceState::waitingForPaymentResult:
    l.i() << "changed state to: waitingForPaymentResult" << Logger::endl;
    break;
  case AutoDeviceState::waitingForPaymentToken:
    l.i() << "changed state to: waitingForPaymentToken" << Logger::endl;
    break;
  }
}

void onPaymentError(const PaymentError& error) {
  l.e() << "payment error!" << Logger::endl;
}

void onMobileEvent(const JetBeep::SerialMobileEvent& event) {
  if (event == SerialMobileEvent::connected) {
    l.i() << "connected" << Logger::endl;
  } else {
    l.e() << "disconnected" << Logger::endl;
  }
}

std::string random_string() {
  std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
  std::random_device rd;
  std::mt19937 generator(rd());
  std::shuffle(str.begin(), str.end(), generator);
  return str.substr(0, 16);
}

int main() {
  Logger::coutEnabled = true;
  Logger::level = LoggerLevel::verbose;

  JetBeep::AutoDevice device;
  device.paymentErrorCallback = onPaymentError;
  device.mobileCallback = onMobileEvent;
  device.stateCallback = onStateChange;
  device.start();

  auto backend = EasyPayBackend(EasyPayHostEnv::Development, MERCHANT_SECRET_KEY);

  const string merchantTransactionId = random_string();
  const int amountInCoins = 5;
  const PaymentMetadata metadata; // empty

  cout << "Device connected?\n";
  string input;
  getline(cin, input);

  auto state = device.state();
  if (state == AutoDeviceState::sessionClosed) {
    device.openSession();
  }

  auto onRequestErrors = [=](const std::exception_ptr error) {
    try {
      rethrow_exception(error);
    } catch (JetBeep::HttpErrors::NetworkError& e) {
      l.e() << e.what() << Logger::endl;
    } catch (JetBeep::HttpErrors::ServerError& e) {
      l.e() << e.what() << " status code: " << e.statusCode << Logger::endl;
    } catch (JetBeep::HttpErrors::RequestError& e) {
      l.e() << "request error:" << e.getRequestError() << Logger::endl;
    } catch (JetBeep::HttpErrors::APIError& e) {
      l.e() << "API error:" << e.what() << Logger::endl;
    } catch (std::exception& e) {
      l.e() << e.what() << Logger::endl;
    }
  };

  auto onRefundResult = [&](EasyPayResult result) {
    l.i() << "refund status: " << (int)result.Status << Logger::endl;
  };

  auto onPaymentStatusGet = [&](EasyPayResult result) {
    l.i() << "payment status: " << (int)result.Status << Logger::endl;

    backend.makeRefund(result.TransactionId, amountInCoins, DEVICE_ID)
    .then(onRefundResult)
    .catchError(onRequestErrors);
  };

  auto onPaymentSuccess = [&](EasyPayResult result) {
    l.i() << "PAYMENT SUCCESS" << Logger::endl;

    backend.getPaymentStatus(merchantTransactionId, amountInCoins, DEVICE_ID)
    .then(onPaymentStatusGet)
    .catchError(onRequestErrors);
  };

  device.createPaymentToken(amountInCoins, merchantTransactionId, CASHIER_ID, metadata).then([&](string fullToken) {
    l.i() << "Payment token: " << fullToken << Logger::endl;
    backend.makePayment(merchantTransactionId, fullToken, amountInCoins, DEVICE_ID, CASHIER_ID)
      .then([=](EasyPayResult result) {
        // l.i() << "Request result: " << result._rawResponse << Logger::endl;
        if (result.Status == EasyPayAPI::PaymentStatus::Accepted) {
          return onPaymentSuccess(result);
        } else {
          l.i() << "PAYMENT status:" << (int)result.Status << Logger::endl;
        }
      })
      .catchError(onRequestErrors);
  });

  while (true) {
    getline(cin, input);
  }
  return 0;
}