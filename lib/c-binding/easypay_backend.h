#ifndef JETBEEP_C_EASYPAY_BACKEND_H
#define JETBEEP_C_EASYPAY_BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"

typedef void* jetbeep_easypay_handle_t;

typedef enum {
  EASYPAY_BACKEND_DEVELOPMENT = 0,
  EASYPAY_BACKEND_PRODUCTION = 1,
} jetbeep_easypay_environment_t;

typedef struct {
  const char* error_string;
  long easypay_transaction_id;
  const char* easypay_payment_request_uid;
} jetbeep_easypay_payment_result_t;

typedef struct {
  const char* error_string;
} jetbeep_easypay_refund_result_t;

typedef void (*jetbeep_easypay_payment_result_cb)(jetbeep_easypay_payment_result_t result, void* data);
typedef void (*jetbeep_easypay_refund_result_cb)(jetbeep_easypay_refund_result_t result, void* data);

JETBEEP_API jetbeep_easypay_handle_t jetbeep_easypay_new(jetbeep_easypay_environment_t environment, const char* merchant_secret_key);
JETBEEP_API void jetbeep_easypay_free(jetbeep_easypay_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_easypay_make_payment(jetbeep_easypay_handle_t handle,
                                                         const char* merchant_transaction_id,
                                                         const char* payment_token,
                                                         uint32_t amount_in_coins,
                                                         uint32_t device_id,
                                                         jetbeep_easypay_payment_result_cb callback,
                                                         void* data,
                                                         const char* cashier_id = "unspecified");

JETBEEP_API jetbeep_error_t jetbeep_easypay_make_payment_partials(jetbeep_easypay_handle_t handle,
                                                                  const char* merchant_transaction_id,
                                                                  const char* payment_token,
                                                                  uint32_t amount_in_coins,
                                                                  uint32_t device_id,
                                                                  jetbeep_payment_metadata_t* metadata,
                                                                  size_t metadata_size,
                                                                  jetbeep_easypay_payment_result_cb callback,
                                                                  void* data,
                                                                  const char* cashier_id = "unspecified");
JETBEEP_API jetbeep_error_t jetbeep_easypay_make_refund(jetbeep_easypay_handle_t handle,
                                                        long easypay_transaction_id,
                                                        uint32_t amount_in_coins,
                                                        uint32_t device_id,
                                                        jetbeep_easypay_refund_result_cb callback,
                                                        void* data);
JETBEEP_API jetbeep_error_t jetbeep_easypay_make_refund_partials(jetbeep_easypay_handle_t handle,
                                                                 const char* payment_request_uid,
                                                                 uint32_t amount_in_coins,
                                                                 uint32_t device_id,
                                                                 jetbeep_easypay_refund_result_cb callback,
                                                                 void* data);

#ifdef __cplusplus
}
#endif

#endif