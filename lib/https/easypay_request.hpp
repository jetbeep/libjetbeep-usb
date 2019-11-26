#ifndef EASYPAY_REQUEST
#define EASYPAY_REQUEST

#include <string>
#include <vector>

using namespace std;

namespace JetBeep::EasyPayAPI {
  typedef struct {
    string date;
    string signature;
  } RequestSignature;

  typedef struct {
    uint32_t AmountInCoin;
    string SignatureMerchant;
    string PaymentTokenFull;
    string DateRequest;
    string MerchantCashboxId;
    string MerchantTransactionId;
  } TokenPaymentRequest;

  typedef struct {
    string DateRequest;
    string SignatureMerchant;
    uint32_t AmountInCoin;
    string MerchantTransactionId;
  } TokenGetStatusRequest;

  typedef struct {
    string DateRequest;
    string SignatureMerchant;
    uint32_t AmountInCoin;
    long TransactionId; //note: this is PSP transaction ID
  } TokenRefundRequest;

  string tokenPaymentReqToJSON(TokenPaymentRequest& data);

  string tokenRefundReqToJSON(TokenRefundRequest& data);

  string tokenGetStatusReqToJSON(TokenGetStatusRequest& data);

} // namespace JetBeep::EasyPayAPI

#endif