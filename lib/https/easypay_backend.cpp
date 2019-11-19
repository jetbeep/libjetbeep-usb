#include "./easypay_backend.hpp"
#include <iostream>

using namespace JetBeep;
using namespace std;

struct EasyPayBackend::Impl {
  Impl(string serverHost, int port = 8193) : serverHost(serverHost), port(port){};

  ~Impl();

  Promise<EasyPayResult> makePayment(string paymentToken);

  Promise<EasyPayResult> getPaymentStatus(string pspTransactionId);

  Promise<EasyPayResult> makeRefund(string pspTransactionId);

  string serverHost;
  int port;
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
  throw runtime_error("not implemented");
}

Promise<EasyPayResult> EasyPayBackend::Impl::getPaymentStatus(string pspTransactionId) {
  const string path = "/api/Payment/GetStatusTransaction"; // GET
  throw runtime_error("not implemented");
}

Promise<EasyPayResult> EasyPayBackend::Impl::makeRefund(string pspTransactionId) {
  const string path = "/api/Payment/Refund"; // POST
  throw runtime_error("not implemented");
}