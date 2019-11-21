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

  Promise<EasyPayResult> makePayment(string merchantTransactionId, string paymentToken, uint32_t amountInCoins, uint32_t deviceId, string cashierId);

  Promise<EasyPayResult> getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId);

  Promise<EasyPayResult> makeRefund(string pspTransactionId, uint32_t amountInCoins, uint32_t deviceId);

private:
  Https::HttpsClient m_httpsClient;
  string m_serverHost;
  string m_merchantSecretKey;
  int m_port;
  Logger m_log;

  string makeMerchantSignature(string date, uint32_t deviceId);
};

EasyPayBackend::Impl::~Impl() = default;

EasyPayBackend::EasyPayBackend(EasyPayHostEnv env, string merchantSecretKey)
  : m_impl(new Impl(env == EasyPayHostEnv::Production ? "sastest.easypay.ua" : "sas.easypay.ua", merchantSecretKey)) {
}

EasyPayBackend::~EasyPayBackend() = default;

Promise<EasyPayResult> EasyPayBackend::makePayment(
  string merchantTransactionId, string paymentToken, uint32_t amountInCoins, uint32_t deviceId, string cashierId) {
  return m_impl->makePayment(merchantTransactionId, paymentToken, amountInCoins, deviceId, cashierId);
}

Promise<EasyPayResult> EasyPayBackend::getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  return m_impl->getPaymentStatus(merchantTransactionId, amountInCoins, deviceId);
}

Promise<EasyPayResult> EasyPayBackend::makeRefund(string pspTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  return m_impl->makeRefund(pspTransactionId, amountInCoins, deviceId);
}
Promise<EasyPayResult> EasyPayBackend::Impl::makePayment(
  string merchantTransactionId, string paymentToken, uint32_t amountInCoins, uint32_t deviceId, string cashierId) {
  const string path = "/api/Payment/Box";
  TokenPaymentRequest data;

  data.AmountInCoin = amountInCoins;
  data.DateRequest = "2018-06-26T06:53:03.691Z"; // TODO dates
  data.MerchantCashboxId = cashierId;
  data.MerchantTransactionId = merchantTransactionId;
  data.PaymentTokenFull = paymentToken;
  data.SignatureMerchant = makeMerchantSignature(data.DateRequest, deviceId);

  string reqBody = tokenPaymentReqToJSON(data);

  Https::RequestOptions options;
  options.method = Https::RequestMethod::POST;
  options.contentType = Https::RequestContentType::JSON;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.body = reqBody;

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([=](Https::Response res) {
    auto promise = Promise<EasyPayResult>();
    if (res.isHttpError) {
      promise.reject(make_exception_ptr(HttpErrors::ServerError(res.statusCode)));
      return promise;
    }
    try {
      auto result = parseTokenPaymentResult(res.body);
      result._rawResponse = res.body;
      result.statusCode = res.statusCode;
      if (result.isError()) {
        promise.reject(make_exception_ptr(HttpErrors::RequestError(result.primaryErrorMsg)));
        return promise;
      }
      promise.resolve(result);
    } catch (...) {
      promise.reject(make_exception_ptr(HttpErrors::APIError()));
    }
  });
}

Promise<EasyPayResult> EasyPayBackend::Impl::getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  const string path = "/api/Payment/GetStatusTransaction";
  TokenGetStatusRequest data;

  data.AmountInCoin = amountInCoins;
  data.DateRequest = "2018-06-26T06:53:03.691Z"; // TODO dates
  data.MerchantTransactionId = merchantTransactionId;
  data.SignatureMerchant = makeMerchantSignature(data.DateRequest, deviceId);

  string reqBody = tokenGetStatusReqToJSON(data);

  Https::RequestOptions options;
  options.method = Https::RequestMethod::GET;
  options.contentType = Https::RequestContentType::JSON;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.body = reqBody;

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([=](Https::Response res) {
    auto promise = Promise<EasyPayResult>();
    if (res.isHttpError) {
      promise.reject(make_exception_ptr(HttpErrors::ServerError(res.statusCode)));
      return promise;
    }
    try {
      auto result = parseTokenGetStatusResult(res.body);
      result._rawResponse = res.body;
      result.statusCode = res.statusCode;

      if (result.isError()) {
        promise.reject(make_exception_ptr(HttpErrors::RequestError(result.primaryErrorMsg)));
        return promise;
      }

      promise.resolve(result);
    } catch (...) {
      promise.reject(make_exception_ptr(HttpErrors::APIError()));
    }
    return promise;
  });
}

Promise<EasyPayResult> EasyPayBackend::Impl::makeRefund(string pspTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  const string path = "/api/Payment/Refund";
  TokenRefundRequest data;

  data.AmountInCoin = amountInCoins;
  data.DateRequest = "2018-06-26T06:53:03.691Z"; // TODO dates
  data.TransactionId = pspTransactionId;
  data.SignatureMerchant = makeMerchantSignature(data.DateRequest, deviceId);

  string reqBody = tokenRefundReqToJSON(data);

  Https::RequestOptions options;
  options.method = Https::RequestMethod::POST;
  options.contentType = Https::RequestContentType::JSON;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.body = reqBody;

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([=](Https::Response res) {
    auto promise = Promise<EasyPayResult>();
    if (res.isHttpError) {
      promise.reject(make_exception_ptr(HttpErrors::ServerError(res.statusCode)));
      return promise;
    }
    try {
      auto result = parseTokenRefundResult(res.body);
      result._rawResponse = res.body;
      result.statusCode = res.statusCode;
      if (result.isError()) {
        promise.reject(make_exception_ptr(HttpErrors::RequestError(result.primaryErrorMsg)));
        return promise;
      }
      promise.resolve(result);
    } catch (...) {
      promise.reject(make_exception_ptr(HttpErrors::APIError()));
    }
    return promise;
  });
}

string EasyPayBackend::Impl::makeMerchantSignature(string date, uint32_t deviceId) {
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