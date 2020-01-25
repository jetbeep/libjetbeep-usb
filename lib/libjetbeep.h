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
#define JETBEEP_ERROR_INVALID_STATE 1
#define JETBEEP_ERROR_IO 2
#define JETBEEP_ERROR_PAYMENT_NETWORK 100
#define JETBEEP_ERROR_PAYMENT_TIMEOUT 101
#define JETBEEP_ERROR_PAYMENT_SERVER 102
#define JETBEEP_ERROR_PAYMENT_SECURITY 103
#define JETBEEP_ERROR_PAYMENT_WITHDRAWAL 104
#define JETBEEP_ERROR_PAYMENT_DISCARDED 105
#define JETBEEP_ERROR_PAYMENT_UNKNOWN 106
#define JETBEEP_ERROR_PAYMENT_INVALID_PIN 107

#define JETBEEP_STATE_INVALID 0
#define JETBEEP_STATE_FIRMWARE_VERSION_NOT_SUPPORTED 1
#define JETBEEP_STATE_SESSION_OPENED 2
#define JETBEEP_STATE_SESSION_CLOSED 3
#define JETBEEP_STATE_WAITING_FOR_BARCODES 4
#define JETBEEP_STATE_WAITING_FOR_PAYMENT_RESULT 5
#define JETBEEP_STATE_WAITING_FOR_CONFIRMATION 6
#define JETBEEP_STATE_WAITING_FOR_PAYMENT_TOKEN 7

typedef void* jetbeep_autodevice_handle_t;
typedef int jetbeep_error_t;
typedef int jetbeep_state_t;

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

typedef void (*jetbeep_autodevice_barcode_result_cb)(jetbeep_error_t error, jetbeep_barcode_t* barcodes, size_t size);
typedef void (*jetbeep_autodevice_payment_result_cb)(jetbeep_error_t error);
typedef void (*jetbeep_autodevice_payment_token_result_cb)(jetbeep_error_t error, const char* token, size_t size);
typedef void (*jetbeep_autodevice_payment_error_cb)(jetbeep_error_t error);
typedef void (*jetbeep_autodevice_state_cb)(jetbeep_state_t state);
typedef void (*jetbeep_autodevice_mobile_connected_cb)(bool connected);

JETBEEP_API jetbeep_autodevice_handle_t jetbeep_autodevice_new();
JETBEEP_API void jetbeep_autodevice_free(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_start(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_stop(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_open_session(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_close_session(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_request_barcodes(jetbeep_autodevice_handle_t handle,
                                                                jetbeep_autodevice_barcode_result_cb callback);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_cancel_barcodes(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_create_payment(jetbeep_autodevice_handle_t handle,
                                                              uint32_t amount,
                                                              const char* transaction_id,
                                                              size_t transaction_id_size,
                                                              jetbeep_autodevice_payment_result_cb callback,
                                                              const char* cashier_id = "",
                                                              size_t cashier_id_size = 0,
                                                              const jetbeep_payment_metadata_t* metadata = NULL,
                                                              size_t metadata_size = 0);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_confirm_payment(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_create_payment_token(jetbeep_autodevice_handle_t handle,
                                                                    uint32_t amount,
                                                                    const char* transaction_id,
                                                                    size_t transaction_id_size,
                                                                    jetbeep_autodevice_payment_token_result_cb callback,
                                                                    const char* cashier_id = "",
                                                                    size_t cashier_id_size = 0,
                                                                    const jetbeep_payment_metadata_t* metadata = NULL,
                                                                    size_t metadata_size = 0);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_cancel_payment(jetbeep_autodevice_handle_t handle);
JETBEEP_API bool jetbeep_autodevice_is_mobile_connected(jetbeep_autodevice_handle_t handle);
JETBEEP_API const char* jetbeep_autodevice_version(jetbeep_autodevice_handle_t handle);
JETBEEP_API unsigned long jetbeep_autodevice_device_id(jetbeep_autodevice_handle_t handle);
JETBEEP_API void* jetbeep_autodevice_get_opaque(jetbeep_autodevice_handle_t handle);
JETBEEP_API void jetbeep_autodevice_set_opaque(jetbeep_autodevice_handle_t handle, void* opaque);
JETBEEP_API jetbeep_state_t jetbeep_autodevice_state(jetbeep_autodevice_handle_t handle);
JETBEEP_API void jetbeep_autodevice_set_payment_error_callback(jetbeep_autodevice_handle_t handle,
                                                               jetbeep_autodevice_payment_error_cb callback);
JETBEEP_API void jetbeep_autodevice_set_state_callback(jetbeep_autodevice_handle_t handle, jetbeep_autodevice_state_cb callback);
JETBEEP_API void jetbeep_autodevice_set_mobile_connected_callback(jetbeep_autodevice_handle_t handle,
                                                                  jetbeep_autodevice_mobile_connected_cb callback);

#ifdef __cplusplus
}
#endif

#endif
