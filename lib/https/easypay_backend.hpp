#ifndef EASYPAY_BACKEND_HPP
#define EASYPAY_BACKEND_HPP

#include "../io/iocontext.hpp"
#include "../utils/promise.hpp"
#include "./http_errors.hpp"
#include <memory>
#include <string>
#include <vector>

using namespace std;

namespace JetBeep {

  enum class EasyPayHostEnv { Development, Production };

  typedef struct {
    string Error;
    long CodeId;
    string CodeName;
    string ErrorMessage;
    string UserMessage;
    string Reason;
  } EasyPayError;

  class EasyPayResult {
  public:
    string Uid;
    vector<EasyPayError> Errors;
    string _rawResponse;
    int statusCode;

    bool isError() {
      return m_errorsCount > 0;
    }

  private:
    int m_errorsCount = 0;
  };

  class EasyPayBackend {
  public:
    ~EasyPayBackend();
    EasyPayBackend(EasyPayHostEnv env, string merchantSecretKey);

    Promise<EasyPayResult> makePayment(string paymentToken);

    Promise<EasyPayResult> getPaymentStatus(string pspTransactionId);

    Promise<EasyPayResult> makeRefund(string pspTransactionId);

  private:
    class Impl;
    unique_ptr<Impl> m_impl;
  };

} // namespace JetBeep

#endif