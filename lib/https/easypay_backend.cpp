#include "../utils/platform.hpp"
#include "./easypay_backend.hpp"
#include "../utils/logger.hpp"
#include "./https_client.hpp"
#include "../utils/cryptlite/sha256.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <time.h>

using namespace JetBeep;
using namespace std;

struct EasyPayBackend::Impl {
  Impl(string serverHost, string merchantSecretKey, int port = 8193)
    : m_serverHost(serverHost), m_port(port), m_log("backend"), m_merchantSecretKey(merchantSecretKey){};

  ~Impl();

  Promise<EasyPayResult> makePayment(string merchantTransactionId, string paymentToken, uint32_t amountInCoins, uint32_t deviceId, string cashierId);

  Promise<EasyPayResult> getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId);

  Promise<EasyPayResult> makeRefund(long pspTransactionId, uint32_t amountInCoins, uint32_t deviceId);

private:
  Https::HttpsClient m_httpsClient;
  string m_serverHost;
  string m_merchantSecretKey;
  int m_port;
  Logger m_log;

  RequestSignature makeMerchantSignature(uint32_t deviceId);
};

EasyPayBackend::Impl::~Impl() = default;

EasyPayBackend::EasyPayBackend(EasyPayHostEnv env, string merchantSecretKey)
  : m_impl(new Impl(env == EasyPayHostEnv::Production ? "sas.easypay.ua" : "sastest.easypay.ua", merchantSecretKey)) {
}

EasyPayBackend::~EasyPayBackend() = default;

Promise<EasyPayResult> EasyPayBackend::makePayment(
  string merchantTransactionId, string paymentToken, uint32_t amountInCoins, uint32_t deviceId, string cashierId) {
  return m_impl->makePayment(merchantTransactionId, paymentToken, amountInCoins, deviceId, cashierId);
}

Promise<EasyPayResult> EasyPayBackend::getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  return m_impl->getPaymentStatus(merchantTransactionId, amountInCoins, deviceId);
}

Promise<EasyPayResult> EasyPayBackend::makeRefund(long pspTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  return m_impl->makeRefund(pspTransactionId, amountInCoins, deviceId);
}

Promise<EasyPayResult> EasyPayBackend::Impl::makePayment(
  string merchantTransactionId, string paymentToken, uint32_t amountInCoins, uint32_t deviceId, string cashierId) {
  const string path = "/api/Payment/Box";
  TokenPaymentRequest data;
  auto sigData = makeMerchantSignature(deviceId);

  data.AmountInCoin = amountInCoins;
  data.DateRequest = sigData.date;
  data.MerchantCashboxId = cashierId;
  data.MerchantTransactionId = merchantTransactionId;
  data.PaymentTokenFull = paymentToken;
  data.SignatureMerchant = sigData.signature;

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
    return promise;
  });  
}

Promise<EasyPayResult> EasyPayBackend::Impl::getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  const string path = "/api/Payment/GetStatusTransaction";
  TokenGetStatusRequest data;

  auto sigData = makeMerchantSignature(deviceId);

  data.AmountInCoin = amountInCoins;
  data.DateRequest = sigData.date;
  data.MerchantTransactionId = merchantTransactionId;
  data.SignatureMerchant = sigData.signature;

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

Promise<EasyPayResult> EasyPayBackend::Impl::makeRefund(long pspTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  const string path = "/api/Payment/Refund";
  TokenRefundRequest data;
  auto sigData = makeMerchantSignature(deviceId);

  data.AmountInCoin = amountInCoins;
  data.DateRequest = sigData.date;
  data.TransactionId = pspTransactionId;
  data.SignatureMerchant = sigData.signature;

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

RequestSignature EasyPayBackend::Impl::makeMerchantSignature(uint32_t deviceId) {
  RequestSignature sigFields;
  
  //current time
  std::time_t now = std::time(NULL);
  std::tm* p_now = std::gmtime(&now);
  stringstream isoNowStream;
  isoNowStream << std::put_time(p_now, "%FT%TZ");
  sigFields.date = isoNowStream.str();


  //easyPay point in time 1988-06-27T22:15:00Z
  struct tm m_time;

  m_time.tm_sec = 0;
  m_time.tm_min = 15;
  m_time.tm_hour = 22;
  m_time.tm_mday = 27;
  m_time.tm_mon = 5;
  m_time.tm_year = 1988 - 1900;
  m_time.tm_wday = 1;
  m_time.tm_yday = 178;
  m_time.tm_isdst = -1;
#ifdef PLATFORM_WIN
  std::time_t t_now2 =  _mkgmtime(p_now);    
  std::time_t t_then2 = _mkgmtime(&m_time);  
#else
  m_time.tm_gmtoff = 0;
  m_time.tm_zone = (char*)"GMT";
  std::time_t t_now2 =  timegm(p_now);    
  std::time_t t_then2 = timegm(&m_time);  
#endif

  int secondsDiff = (int)difftime(t_now2, t_then2);

  string body = m_merchantSecretKey + std::to_string(secondsDiff) + std::to_string(deviceId);

  sigFields.signature = cryptlite::sha256::hash_base64(body);

  return sigFields;
}