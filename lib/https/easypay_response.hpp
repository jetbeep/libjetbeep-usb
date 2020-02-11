#ifndef EASYPAY_RESPONSE
#define EASYPAY_RESPONSE

#include <string>
#include <vector>
#include "./https_response.hpp"

using namespace std;

namespace JetBeep::EasyPayAPI {

  enum class PaymentStatus{
    None = 0,
    Inserted,
    Accepted,
    Declined,
    Deleted,
    InProcess,
    Hold,
    Created
  };

  typedef struct {
    string Error;
    long CodeId;
    string CodeName;
    string ErrorMessage;
    string UserMessage;
    string Reason;
  } EasyPayError;

  class EasyPayResult : public HTTPResponseBase {
  public:
    string Uid;
    vector<EasyPayError> Errors;
    string primaryErrorMsg;
    
    //Result
    PaymentStatus Status = PaymentStatus::None;
    long TransactionId = 0;
    string TransactionDatePost = "";
    string PaymentRequestUid = "";
    string MerchantTransactionId = "";

    bool isError() {
      return Errors.size() > 0;
    }
  };

  EasyPayResult parseTokenPaymentResult(const string &json);

  EasyPayResult parseTokenRefundResult(const string &json);

  EasyPayResult parseTokenGetStatusResult(const string &json);
  
} // namespace JetBeep::EasyPayAPI

#endif