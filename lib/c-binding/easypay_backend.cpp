#define JETBEEP_API_EXPORTS
#include "../libjetbeep.h"
#include "../libjetbeep.hpp"

using namespace JetBeep;
using namespace std;

JETBEEP_API jetbeep_easypay_handle_t jetbeep_easypay_new(jetbeep_easypay_environment_t environment, const char* merchant_secret_key) {
  auto secretKey = string(merchant_secret_key);
  return new EasyPayBackend((EasyPayHostEnv)environment, secretKey);
}

JETBEEP_API void jetbeep_easypay_free(jetbeep_easypay_handle_t handle) {
  auto backend = (EasyPayBackend*)handle;
  delete backend;
}

JETBEEP_API jetbeep_error_t jetbeep_easypay_make_payment(jetbeep_easypay_handle_t handle,
                                                         const char* merchant_transaction_id,
                                                         const char* payment_token,
                                                         uint32_t amount_in_coins,
                                                         uint32_t device_id,
                                                         jetbeep_easypay_payment_result_cb callback,
                                                         void* data,
                                                         const char* cashier_id) {
  auto backend = (EasyPayBackend*)handle;
  try {
    auto merchantTransactionId = string(merchant_transaction_id);
    auto paymentToken = string(payment_token);
    auto cashierId = string(cashier_id);

    backend->makePayment(merchantTransactionId, paymentToken, amount_in_coins, device_id, cashierId)
      .then([callback, data](EasyPayResult result) {
        jetbeep_easypay_payment_result_t payment_result;
        if (result.isError()) {
          memset(&payment_result, 0, sizeof(payment_result));
          payment_result.error_string = result.primaryErrorMsg.c_str();
        } else {
          payment_result.easypay_payment_request_uid = result.PaymentRequestUid.c_str();
          payment_result.easypay_transaction_id = result.TransactionId;
          payment_result.error_string = nullptr;
        }
        callback(payment_result, data);
      })
      .catchError([callback, data](const exception_ptr&) {
        jetbeep_easypay_payment_result_t payment_result;
        memset(&payment_result, 0, sizeof(payment_result));
        payment_result.error_string = "Network error";
        callback(payment_result, data);
      });
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_easypay_make_payment_partials(jetbeep_easypay_handle_t handle,
                                                                  const char* merchant_transaction_id,
                                                                  const char* payment_token,
                                                                  uint32_t amount_in_coins,
                                                                  uint32_t device_id,
                                                                  jetbeep_payment_metadata_t* metadata,
                                                                  size_t metadata_size,
                                                                  jetbeep_easypay_payment_result_cb callback,
                                                                  void* data,
                                                                  const char* cashier_id) {
  auto backend = (EasyPayBackend*)handle;
  try {
    auto merchantTransactionId = string(merchant_transaction_id);
    auto paymentToken = string(payment_token);
    auto cashierId = string(cashier_id);
    PaymentMetadata metaData;

    for (size_t i = 0; i < metadata_size; ++i) {
      auto key = string(metadata[i].key);
      auto value = string(metadata[i].value);
      metaData[key] = value;
    }

    backend->makePaymentPartials(merchantTransactionId, paymentToken, amount_in_coins, device_id, metaData, cashierId)
      .then([callback, data](EasyPayResult result) {
        jetbeep_easypay_payment_result_t payment_result;
        if (result.isError()) {
          memset(&payment_result, 0, sizeof(payment_result));
          payment_result.error_string = result.primaryErrorMsg.c_str();
        } else {
          payment_result.easypay_payment_request_uid = result.PaymentRequestUid.c_str();
          payment_result.easypay_transaction_id = result.TransactionId;
          payment_result.error_string = nullptr;
        }
        callback(payment_result, data);
      })
      .catchError([callback, data](const exception_ptr&) {
        jetbeep_easypay_payment_result_t payment_result;
        memset(&payment_result, 0, sizeof(payment_result));
        payment_result.error_string = "Network error";
        callback(payment_result, data);
      });
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_easypay_make_refund(jetbeep_easypay_handle_t handle,
                                                        long easypay_transaction_id,
                                                        uint32_t amount_in_coins,
                                                        uint32_t device_id,
                                                        jetbeep_easypay_refund_result_cb callback,
                                                        void* data) {
  auto backend = (EasyPayBackend*)handle;
  try {
    backend->makeRefund(easypay_transaction_id, amount_in_coins, device_id)
      .then([callback, data](EasyPayResult result) {
        jetbeep_easypay_refund_result_t refund_result;

        if (result.isError()) {
          refund_result.error_string = result.primaryErrorMsg.c_str();
        } else {
          refund_result.error_string = nullptr;
        }
        callback(refund_result, data);
      })
      .catchError([callback, data](exception_ptr) {
        jetbeep_easypay_refund_result_t refund_result;

        refund_result.error_string = "Network error";
        callback(refund_result, data);
      });
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_easypay_make_refund_partials(jetbeep_easypay_handle_t handle,
                                                                 const char* payment_request_uid,
                                                                 uint32_t amount_in_coins,
                                                                 uint32_t device_id,
                                                                 jetbeep_easypay_refund_result_cb callback,
                                                                 void* data) {
  auto backend = (EasyPayBackend*)handle;
  try {
    auto paymentRequestId = string(payment_request_uid);
    backend->makeRefundPartials(payment_request_uid, amount_in_coins, device_id)
      .then([callback, data](EasyPayResult result) {
        jetbeep_easypay_refund_result_t refund_result;

        if (result.isError()) {
          refund_result.error_string = result.primaryErrorMsg.c_str();
        } else {
          refund_result.error_string = nullptr;
        }
        callback(refund_result, data);
      })
      .catchError([callback, data](exception_ptr) {
        jetbeep_easypay_refund_result_t refund_result;

        refund_result.error_string = "Network error";
        callback(refund_result, data);
      });
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}