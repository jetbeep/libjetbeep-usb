#include "./easypay_backend.hpp"
#include "../utils/logger.hpp"
#include "./https_client.hpp"
#include <iostream>

using namespace JetBeep;
using namespace std;

struct EasyPayBackend::Impl {
  Impl(string serverHost, string merchantSecretKey, int port = 8193)
    : m_serverHost(serverHost), m_port(port), m_log("backend"), m_merchantSecretKey(merchantSecretKey){};

  ~Impl();

  Promise<EasyPayResult> makePayment(string paymentToken);

  Promise<EasyPayResult> getPaymentStatus(string pspTransactionId);

  Promise<EasyPayResult> makeRefund(string pspTransactionId);

private:
  Https::HttpsClient m_httpsClient;
  string m_serverHost;
  string m_merchantSecretKey;
  int m_port;
  Logger m_log;

  string makeMerchantSignature();
};

EasyPayBackend::Impl::~Impl() = default;

EasyPayBackend::EasyPayBackend(EasyPayHostEnv env, string merchantSecretKey)
  : m_impl(new Impl(env == EasyPayHostEnv::Production ? "sastest.easypay.ua" : "sas.easypay.ua", merchantSecretKey)) {
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

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([=](Https::Response res) {
    EasyPayResult result;
    result._rawResponse = res.body;
    result.statusCode = res.statusCode;
    auto promise = Promise<EasyPayResult>();
    if (res.isHttpError) {
      promise.reject(make_exception_ptr(HttpErrors::ServerError(res.statusCode)));
    } else {
      promise.resolve(result);
    }
    return promise;
  });
}

Promise<EasyPayResult> EasyPayBackend::Impl::getPaymentStatus(string pspTransactionId) {
  const string path = "/api/Payment/GetStatusTransaction"; // GET
  string reqBody = R"JSON(
    {
        "DateRequest": "2019-01-01T01:01:01Z",
        "SignatureMerchant": "string",
        "AmountInCoin": 1000,
        "MerchantTransactionId": "string"
    }
  )JSON";
  Https::RequestOptions options;
  options.method = Https::RequestMethod::GET;
  options.contentType = Https::RequestContentType::JSON;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.body = reqBody;

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([=](Https::Response res) {
    EasyPayResult result;
    result._rawResponse = res.body;
    result.statusCode = res.statusCode;
    auto promise = Promise<EasyPayResult>();
    if (res.isHttpError) {
      promise.reject(make_exception_ptr(HttpErrors::ServerError(res.statusCode)));
    } else {
      promise.resolve(result);
    }
    return promise;
  });
}

Promise<EasyPayResult> EasyPayBackend::Impl::makeRefund(string pspTransactionId) {
  const string path = "/api/Payment/Refund"; // POST
  string reqBody = R"JSON(
    {
        "DateRequest": "2019-01-01T01:01:01Z",
        "SignatureMerchant": "string",
        "MerchantTransactionId": "string"
    }
  )JSON";
  Https::RequestOptions options;
  options.method = Https::RequestMethod::POST;
  options.contentType = Https::RequestContentType::JSON;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.body = reqBody;

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([=](Https::Response res) {
    EasyPayResult result;
    result._rawResponse = res.body;
    result.statusCode = res.statusCode;
    auto promise = Promise<EasyPayResult>();
    if (res.isHttpError) {
      promise.reject(make_exception_ptr(HttpErrors::ServerError(res.statusCode)));
    } else {
      promise.resolve(result);
    }
    return promise;
  });
}

string EasyPayBackend::Impl::makeMerchantSignature() {
  // TODO
  /*
  function createMerchantSignature(paymentData, secretkey) {
  const firstDate = moment.utc([1988, 5 /* 0 based *//*, 27, 22, 15, 0, 0]);
  const nowDate = moment.utc(paymentData.datePaymentStart);
  const secondsDiff = nowDate.diff(firstDate, 'seconds');

  let crypto = null;
  try {
    Buffer.from([]);
    crypto = window.require('crypto');
  } catch (er) {
    console.warn(er);
    window.alert('This tool does not work on browser environment');
    return null;
  }

  const body = Buffer.from(`${secretkey}${secondsDiff}${paymentData.deviceId}`, 'utf8');

  return crypto.createHash('sha256').update(body).digest('base64');
}
  */

  return "test";
}