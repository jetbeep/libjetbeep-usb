#ifndef EASYPAY_BACKEND_HPP
#define EASYPAY_BACKEND_HPP

#include "../io/iocontext.hpp"
#include "../utils/promise.hpp"
#include <memory>
#include <string>

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
    string Uid;
    EasyPayError Errors[10];

    bool isError() {
      return m_errorsCount > 0;
    }

  private:
    int m_errorsCount = 0;
  };

  class EasyPayBackend {
  public:
    ~EasyPayBackend();
    EasyPayBackend(EasyPayHostEnv env);

    Promise<EasyPayResult> makePayment(string paymentToken);

    Promise<EasyPayResult> getPaymentStatus(string pspTransactionId);

    Promise<EasyPayResult> makeRefund(string pspTransactionId);

  private:
    class Impl;
    unique_ptr<Impl> m_impl;
  };

} // namespace JetBeep

#endif