#ifndef EASYPAY_BACKEND_HPP
#define EASYPAY_BACKEND_HPP

#include "../io/iocontext.hpp"
#include "../utils/promise.hpp"
#include "./easypay_request.hpp"
#include "./easypay_response.hpp"
#include "./http_errors.hpp"

#include <memory>

using namespace std;

namespace JetBeep {

  using namespace EasyPayAPI;

  enum class EasyPayHostEnv : int { Development = 0, Production = 1 };
  
  enum class TransactionEndpointType { SIMPLE, PARTIALS };

  class EasyPayBackend {
  public:
    ~EasyPayBackend();
    EasyPayBackend(EasyPayHostEnv env, string merchantSecretKey, IOContext context = IOContext::context);

    Promise<EasyPayResult> makePayment(string merchantTransactionId,
                                       string paymentToken,
                                       uint32_t amountInCoins,
                                       uint32_t deviceId,
                                       string cashierId = "unspecified");

    Promise<EasyPayResult> makePaymentPartials(string merchantTransactionId,
                                       string paymentToken,
                                       uint32_t amountInCoins,
                                       uint32_t deviceId,
                                       PaymentMetadata metadata,
                                       string cashierId = "unspecified");                                   

    Promise<EasyPayResult> getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId);

    Promise<EasyPayResult> makeRefund(long pspTransactionId, uint32_t amountInCoins, uint32_t deviceId);

    Promise<EasyPayResult> makeRefundPartials(string pspPaymentRequestUid, uint32_t amountInCoins, uint32_t deviceId);

    void* opaque;

  private:
    class Impl;
    unique_ptr<Impl> m_impl;
  };

} // namespace JetBeep

#endif