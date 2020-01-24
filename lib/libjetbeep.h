#ifndef LIBJETBEEP_H
#define LIBJETBEEP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#ifdef JETBEEP_API_EXPORTS
#define JETBEEP_API __declspec(dllexport)
#else
#define JETBEEP_API __declspec(dllimport)
#endif
#else
#define JETBEEP_API
#endif

#define JETBEEP_NO_ERROR 0
#define JETBEEP_ERROR_INVALID_ARGUMENT 1

typedef void* jetbeep_autodevice_handle_t;
typedef int jetbeep_error_t;

typedef struct {
  const char* barcode;
  size_t barcode_size;
  int type;
} jetbeep_barcode_t;

typedef struct {
  const char* key;
  size_t key_size;
  const char* value;
  size_t value_size;
} jetbeep_payment_metadata_t;

typedef void (*jetbeep_barcode_result_cb)(jetbeep_error_t error, jetbeep_barcode_t* barcodes, size_t size);
typedef void (*jetbeep_payment_result_cb)();
typedef void (*jetbeep_payment_token_result_cb)(const char* token, size_t size);

JETBEEP_API jetbeep_autodevice_handle_t jetbeep_autodevice_new();
JETBEEP_API void jetbeep_autodevice_free(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_start(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_stop(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_open_session(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_close_session(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_request_barcodes(jetbeep_autodevice_handle_t handle, jetbeep_barcode_result_cb callback);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_cancel_barcodes(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_create_payment(jetbeep_autodevice_handle_t handle,
                                                              uint32_t amount,
                                                              const char* transaction_id,
                                                              size_t transaction_id_size,
                                                              jetbeep_payment_result_cb callback,
                                                              const char* cashier_id = "",
                                                              size_t cashier_id_size = 0,
                                                              const jetbeep_payment_metadata_t* metadata = NULL,
                                                              size_t metadata_size = 0);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_confirm_payment(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_create_payment_token(jetbeep_autodevice_handle_t handle,
                                                                    const char* transaction_id,
                                                                    size_t transaction_id_size,
                                                                    jetbeep_payment_token_result_cb callback,
                                                                    const char* cashier_id = "",
                                                                    size_t cashier_id_size = 0,
                                                                    const jetbeep_payment_metadata_t* metadata = NULL,
                                                                    size_t metadata_size = 0);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_cancel_payment(jetbeep_autodevice_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
