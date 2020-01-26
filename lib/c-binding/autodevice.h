#ifndef JETBEEP_C_AUTODEVICE_H
#define JETBEEP_C_AUTODEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"

typedef void* jetbeep_autodevice_handle_t;
typedef int jetbeep_error_t;
typedef enum {
    JETBEEP_STATE_INVALID = 0, 
    JETBEEP_STATE_FIRMWARE_VERSION_NOT_SUPPORTED,

    JETBEEP_STATE_SESSION_OPENED,
    JETBEEP_STATE_SESSION_CLOSED,
    JETBEEP_STATE_WAITING_FOR_BARCODES,

    JETBEEP_STATE_WAITING_FOR_PAYMENT_RESULT,
    JETBEEP_STATE_WAITING_FOR_CONFIRMATION,

    JETBEEP_STATE_WAITING_FOR_PAYMENT_TOKEN
} jetbeep_state_t;

typedef struct {
  const char* barcode;
  int type;
} jetbeep_barcode_t;


typedef void (*jetbeep_autodevice_barcode_result_cb)(jetbeep_error_t error, jetbeep_barcode_t* barcodes, size_t size);
typedef void (*jetbeep_autodevice_payment_result_cb)(jetbeep_error_t error);
typedef void (*jetbeep_autodevice_payment_token_result_cb)(jetbeep_error_t error, const char* token);
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
                                                              jetbeep_autodevice_payment_result_cb callback,
                                                              const char* cashier_id = "",
                                                              const jetbeep_payment_metadata_t* metadata = NULL,
                                                              size_t metadata_size = 0);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_confirm_payment(jetbeep_autodevice_handle_t handle);
JETBEEP_API jetbeep_error_t jetbeep_autodevice_create_payment_token(jetbeep_autodevice_handle_t handle,
                                                                    uint32_t amount,
                                                                    const char* transaction_id,
                                                                    jetbeep_autodevice_payment_token_result_cb callback,
                                                                    const char* cashier_id = "",
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