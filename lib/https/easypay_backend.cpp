#include "./easypay_backend.hpp"
#include <iostream>
#include "./https_client.hpp"
#include "../utils/logger.hpp"

using namespace JetBeep;
using namespace std;

struct EasyPayBackend::Impl {
  Impl(string serverHost, int port = 8193) 
  : m_serverHost(serverHost), m_port(port), m_log("backend"){};

  ~Impl();

  Promise<EasyPayResult> makePayment(string paymentToken);

  Promise<EasyPayResult> getPaymentStatus(string pspTransactionId);

  Promise<EasyPayResult> makeRefund(string pspTransactionId);

  private:
  Https::HttpsClient m_httpsClient;
  string m_serverHost;
  int m_port;
  Logger m_log;
};

EasyPayBackend::Impl::~Impl() = default;

EasyPayBackend::EasyPayBackend(EasyPayHostEnv env)
  : m_impl(new Impl(env == EasyPayHostEnv::Production ? "sastest.easypay.ua" : "sas.easypay.ua")) {
}

EasyPayBackend::~EasyPayBackend() = default;

Promise<EasyPayResult> EasyPayBackend::makePayment(string paymentToken) {
  return m_impl->makePayment(paymentToken);
}

Promise<EasyPayResult> EasyPayBackend::getPaymentStatus(string pspTransactionId) {
  return m_impl->getPaymentStatus(pspTransactionId);
}

Promise<EasyPayResult> EasyPayBackend::makeRefund(string pspTransactionId) {
  return m_impl->makeRefund(pspTransactionId);
}

Promise<EasyPayResult> EasyPayBackend::Impl::makePayment(string paymentToken) {
  const string path = "/api/Payment/Box"; // POST
  string reqBody = R"JSON(
    {
        "Fields": {
          "DeviceId": "test",
          "MerchantCashboxId": "test",
          "MerchantTransactionId": "test"
        },
        "AmountInCoin": 100500,
        "DateRequest": "yyyy-MM-ddTHH:mm:ssZ"
        "SignatureMerchant": "base64",
        "SignatureBox": "base64",
        "PaymentToken": "base64"
    }
  )JSON";
  Https::RequestOptions options;
  options.method = Https::RequestMethod::POST;
  options.contentType = Https::RequestContentType::JSON;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.body = reqBody;

  return m_httpsClient.request(options)
    .thenPromise<EasyPayResult, Promise>([=](string rawResult){
      EasyPayResult result;
      result._rawResponse = rawResult;
      return Promise<EasyPayResult>(result);
    });
}

Promise<EasyPayResult> EasyPayBackend::Impl::getPaymentStatus(string pspTransactionId) {
  const string path = "/api/Payment/GetStatusTransaction"; // GET
  string reqBody = R"JSON(
    {
        "DateRequest": "2019-01-01T01:01:01Z",
        "SignatureMerchant": "string",
        "AmountInCoin": 1000,
        "MerchantTransactionID": "string"
    }
  )JSON";
  Https::RequestOptions options;
  options.method = Https::RequestMethod::GET;
  options.contentType = Https::RequestContentType::JSON;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.body = reqBody;

  return m_httpsClient.request(options)
    .thenPromise<EasyPayResult, Promise>([=](string rawResult){
      EasyPayResult result;
      result._rawResponse = rawResult;
      return Promise<EasyPayResult>(result);
    });
}

Promise<EasyPayResult> EasyPayBackend::Impl::makeRefund(string pspTransactionId) {
  const string path = "/api/Payment/Refund"; // POST
  string reqBody = R"JSON(
    {
        "DateRequest": "2019-01-01T01:01:01Z",
        "SignatureMerchant": "string",
        "MerchantTransactionID": "string"
    }
  )JSON";
  Https::RequestOptions options;
  options.method = Https::RequestMethod::POST;
  options.contentType = Https::RequestContentType::JSON;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.body = reqBody;

  return m_httpsClient.request(options)
    .thenPromise<EasyPayResult, Promise>([=](string rawResult){
      EasyPayResult result;
      result._rawResponse = rawResult;
      return Promise<EasyPayResult>(result);
    });
}