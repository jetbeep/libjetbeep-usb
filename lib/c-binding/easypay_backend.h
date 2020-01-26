#ifndef JETBEEP_EASYPAY_BACKEND_H
#define JETBEEP_EASYPAY_BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"

typedef void* jetbeep_easypay_backend_handle_t;
typedef struct {
  const char* error_string;
  long easypay_transaction_id;
  const char* easypay_payment_request_uid;
} jetbeep_easypay_backend_payment_result_t;

JETBEEP_API jetbeep_easypay_backend_handle_t jetbeep_easypay_backend_new();
JETBEEP_API void jetbeep_easypay_backend_free(jetbeep_easypay_backend_handle_t handle);
JETBEEP_API void jetbeep_easypay_backend_make_payment(const char* merchant_transaction_id,
                                                      const char* payment_token,
                                                      uint32_t amount_in_coins,
                                                      uint32_t device_id,
                                                      const char* cashier_id = "unspecified");

JETBEEP_API void jetbeep_easypay_backend_make_payment_partials(const char* merchant_transaction_id,
                                                               const char* payment_token,
                                                               uint32_t amount_in_coins,
                                                               uint32_t device_id,
                                                               jetbeep_payment_metadata_t* metadata,
                                                               size_t metadata_size,
                                                               const char* cashier_id = "unspecified");
JETBEEP_API void jetbeep_easypay_backend_make_refund(long easypay_transaction_id, uint32_t amount_in_coins, uint32_t device_id);
JETBEEP_API void jetbeep_easypay_backend_make_refund_partials(const char* payment_request_uid, uint32_t amount_in_coins, uint32_t device_id);

#ifdef __cplusplus
}
#endif

#endif