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

  enum class EasyPayHostEnv { Development, Production };

  class EasyPayBackend {
  public:
    ~EasyPayBackend();
    EasyPayBackend(EasyPayHostEnv env, string merchantSecretKey);

    Promise<EasyPayResult> makePayment(string merchantTransactionId,
                                       string paymentToken,
                                       uint32_t amountInCoins,
                                       uint32_t deviceId,
                                       string cashierId = "unspecified");

    Promise<EasyPayResult> getPaymentStatus(string merchantTransactionId, uint32_t amountInCoins, uint32_t deviceId);

    Promise<EasyPayResult> makeRefund(string pspTransactionId, uint32_t amountInCoins, uint32_t deviceId);

  private:
    class Impl;
    unique_ptr<Impl> m_impl;
  };

} // namespace JetBeep

#endif