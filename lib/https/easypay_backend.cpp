#include "../utils/platform.hpp"
#include "./easypay_backend.hpp"
#include "../utils/logger.hpp"
#include "./https_client/https_client.hpp"
#include "../utils/cryptlite/sha256.h"
#include "../io/iocontext_impl.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <time.h>

using namespace JetBeep;
using namespace std;

class TransactionIdWrap {
public:
  TransactionEndpointType endpointType;
  string PaymentRequestUid;
  long TransactionId;
  TransactionIdWrap(string requestUid)
    : PaymentRequestUid(requestUid), endpointType(TransactionEndpointType::PARTIALS){};
  TransactionIdWrap(long id) : TransactionId(id), endpointType(TransactionEndpointType::SIMPLE){};
};

class EasyPayBackend::Impl {
public:
  Impl(string serverHost, string merchantSecretKey, IOContext context, int port = 8193)
    : m_serverHost(serverHost), m_port(port), m_log("backend"), m_merchantSecretKey(merchantSecretKey), m_context(context){};

  ~Impl();

  Promise<EasyPayResult> makePayment(string merchantTransactionId,
                                     string paymentToken,
                                     uint32_t amountInCoins,
                                     uint32_t deviceId,
                                     PaymentMetadata& metadata,
                                     string cashierId);

  Promise<EasyPayResult> getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId);

  Promise<EasyPayResult> makeRefund(TransactionIdWrap& pspTransactionId, uint32_t amountInCoins, uint32_t deviceId);

private:
  IOContext m_context;
  HttpsClient m_httpsClient;
  string m_serverHost;
  string m_merchantSecretKey;
  int m_port;
  Logger m_log;

  RequestOptions getRequestOptions(string path, RequestMethod method, RequestContentType contentType = RequestContentType::JSON);

  RequestSignature makeMerchantSignature(uint32_t deviceId);
};

EasyPayBackend::Impl::~Impl() = default;

EasyPayBackend::EasyPayBackend(EasyPayHostEnv env, string merchantSecretKey, IOContext context)
  : m_impl(new Impl(env == EasyPayHostEnv::Production ? "sas.easypay.ua" : "sastest.easypay.ua", merchantSecretKey, context)) {
}

EasyPayBackend::~EasyPayBackend() = default;

Promise<EasyPayResult> EasyPayBackend::makePayment(
  string merchantTransactionId, string paymentToken, uint32_t amountInCoins, uint32_t deviceId, string cashierId) {
  auto emptyMetadata = PaymentMetadata();
  return m_impl->makePayment(merchantTransactionId, paymentToken, amountInCoins, deviceId, emptyMetadata, cashierId);
}

Promise<EasyPayResult> EasyPayBackend::makePaymentPartials(string merchantTransactionId,
                                                           string paymentToken,
                                                           uint32_t amountInCoins,
                                                           uint32_t deviceId,
                                                           PaymentMetadata metadata,
                                                           string cashierId) {
  return m_impl->makePayment(merchantTransactionId, paymentToken, amountInCoins, deviceId, metadata, cashierId);
}

Promise<EasyPayResult> EasyPayBackend::getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  return m_impl->getPaymentStatus(merchantTransactionId, amountInCoins, deviceId);
}

Promise<EasyPayResult> EasyPayBackend::makeRefund(long pspTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  auto id = TransactionIdWrap(pspTransactionId);
  return m_impl->makeRefund(id, amountInCoins, deviceId);
}

Promise<EasyPayResult> EasyPayBackend::makeRefundPartials(string pspPaymentRequestUid, uint32_t amountInCoins, uint32_t deviceId) {
  auto id = TransactionIdWrap(pspPaymentRequestUid);
  return m_impl->makeRefund(id, amountInCoins, deviceId);
}

RequestOptions EasyPayBackend::Impl::getRequestOptions(string path, RequestMethod method, RequestContentType contentType) {
  RequestOptions options;
  options.method = method;
  options.contentType = contentType;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.ioContext = m_context;
  return options;
}

Promise<EasyPayResult> EasyPayBackend::Impl::makePayment(string merchantTransactionId,
                                                         string paymentToken,
                                                         uint32_t amountInCoins,
                                                         uint32_t deviceId,
                                                         PaymentMetadata& metadata,
                                                         string cashierId) {
  const string path = metadata.empty() ? "/api/Payment/Box" : 
                                         "/api/Payment/BoxPartialAmounts";
  TokenPaymentRequest data;
  auto sigData = makeMerchantSignature(deviceId);

  data.AmountInCoin = amountInCoins;
  data.DateRequest = sigData.date;
  data.MerchantCashboxId = cashierId;
  data.MerchantTransactionId = merchantTransactionId;
  data.PaymentTokenFull = paymentToken;
  data.SignatureMerchant = sigData.signature;
  data.Metadata = metadata;

  auto options = getRequestOptions(path, RequestMethod::POST);
  options.body = tokenPaymentReqToJSON(data);

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([&](Response res) {
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
  data.DeviceId = deviceId;

  auto options = getRequestOptions(path, RequestMethod::GET);
  options.body = tokenGetStatusReqToJSON(data);

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([=](Response res) {
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

Promise<EasyPayResult> EasyPayBackend::Impl::makeRefund(TransactionIdWrap& pspTransactionId, uint32_t amountInCoins, uint32_t deviceId) {
  const string path = pspTransactionId.endpointType == TransactionEndpointType::SIMPLE ? "/api/Payment/Refund" :
                                                                                         "/api/PartialAmounts/Refund";
  TokenRefundRequest data;
  auto sigData = makeMerchantSignature(deviceId);

  data.AmountInCoin = amountInCoins;
  data.DateRequest = sigData.date;
  data.TransactionId = pspTransactionId.TransactionId;
  data.PaymentRequestUid = pspTransactionId.PaymentRequestUid;
  data.SignatureMerchant = sigData.signature;
  data.DeviceId = deviceId;

  auto options = getRequestOptions(path, RequestMethod::POST);
  options.body = tokenRefundReqToJSON(data);

  return m_httpsClient.request(options).thenPromise<EasyPayResult, Promise>([=](Response res) {
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

  // current time
  std::time_t now = std::time(NULL);
  std::tm* p_now = std::gmtime(&now);
  stringstream isoNowStream;
  isoNowStream << std::put_time(p_now, "%Y-%m-%dT%H:%M:%SZ"); // dont use %F %T due to issues with some compilers (mingw)
  sigFields.date = isoNowStream.str();

  // easyPay point in time 1988-06-27T22:15:00Z
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
  std::time_t t_now2 = _mkgmtime(p_now);
  std::time_t t_then2 = _mkgmtime(&m_time);
#else
  m_time.tm_gmtoff = 0;
  m_time.tm_zone = (char*)"GMT";
  std::time_t t_now2 = timegm(p_now);
  std::time_t t_then2 = timegm(&m_time);
#endif

  int secondsDiff = (int)difftime(t_now2, t_then2);

  string body = m_merchantSecretKey + std::to_string(secondsDiff) + std::to_string(deviceId);

  sigFields.signature = cryptlite::sha256::hash_base64(body);

  return sigFields;
}