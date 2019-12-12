#ifndef EASYPAY_REQUEST
#define EASYPAY_REQUEST

#include <string>
#include <vector>

#include "../device/device_types.hpp"

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
    PaymentMetadata Metadata;
  } TokenPaymentRequest;

  typedef struct {
    uint32_t DeviceId;
    string DateRequest;
    string SignatureMerchant;
    uint32_t AmountInCoin;
    string MerchantTransactionId;
  } TokenGetStatusRequest;

  typedef struct {
    uint32_t DeviceId;
    string DateRequest;
    string SignatureMerchant;
    uint32_t AmountInCoin;
    /* 
      TransactionId is used in case of refund payment without metadata, 
      PaymentRequestUid in case of partials amounts payment (with metadata)
    */
    long TransactionId; //note: TransactionId from psp response 
    string PaymentRequestUid; //note: PaymentRequestUid from psp response
  } TokenRefundRequest;

  string tokenPaymentReqToJSON(TokenPaymentRequest& data);

  string tokenRefundReqToJSON(TokenRefundRequest& data);

  string tokenGetStatusReqToJSON(TokenGetStatusRequest& data);

} // namespace JetBeep::EasyPayAPI

#endif