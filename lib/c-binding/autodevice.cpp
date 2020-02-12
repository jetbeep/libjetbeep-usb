#define JETBEEP_API_EXPORTS
#include "../libjetbeep.h"
#include "../libjetbeep.hpp"
#include <cstring>

using namespace JetBeep;

JETBEEP_API jetbeep_autodevice_handle_t jetbeep_autodevice_new() {
  return (jetbeep_autodevice_handle_t) new AutoDevice;
}

JETBEEP_API void jetbeep_autodevice_free(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  delete autodevice;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_start(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->start();
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_stop(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->stop();
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_open_session(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->openSession();
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_close_session(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->closeSession();
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_request_barcodes(jetbeep_autodevice_handle_t handle,
                                                                jetbeep_autodevice_barcode_result_cb callback,
                                                                void* data) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->requestBarcodes()
      .then([callback, data](vector<Barcode> barcodes) {
        jetbeep_barcode_t* barcodesT = (jetbeep_barcode_t*)malloc(sizeof(jetbeep_barcode_t) * barcodes.size());
        size_t i = 0;
        for (auto it = barcodes.begin(); it != barcodes.end(); ++it) {
          barcodesT[i].barcode = (*it).value.c_str();
          barcodesT[i].type = (int)(*it).type;
          ++i;
        }
        callback(JETBEEP_NO_ERROR, barcodesT, barcodes.size(), data);
        free(barcodesT);
      })
      .catchError([callback, data](exception_ptr error) { callback(JETBEEP_ERROR_IO, nullptr, 0, data); });
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_cancel_barcodes(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->cancelBarcodes();
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_create_payment(jetbeep_autodevice_handle_t handle,
                                                              uint32_t amount,
                                                              const char* transaction_id,
                                                              jetbeep_autodevice_payment_result_cb callback,
                                                              void* data,
                                                              const char* cashier_id,
                                                              const jetbeep_payment_metadata_t* metadata,
                                                              size_t metadata_size) {
  auto autodevice = (AutoDevice*)handle;
  try {
    auto transactionId = string(transaction_id);
    auto cashierId = string(cashier_id);
    PaymentMetadata metaData;

    for (int i = 0; i < metadata_size; ++i) {
      auto key = string(metadata[i].key);
      auto value = string(metadata[i].value);

      metaData[key] = value;
    }

    autodevice->createPayment(amount, transactionId, cashierId, metaData)
      .then([callback, data] { callback(JETBEEP_NO_ERROR, data); })
      .catchError([callback, data](exception_ptr) { callback(JETBEEP_ERROR_IO, data); });
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_confirm_payment(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->confirmPayment();
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_create_payment_token(jetbeep_autodevice_handle_t handle,
                                                                    uint32_t amount,
                                                                    const char* transaction_id,
                                                                    jetbeep_autodevice_payment_token_result_cb callback,
                                                                    void* data,
                                                                    const char* cashier_id,
                                                                    const jetbeep_payment_metadata_t* metadata,
                                                                    size_t metadata_size) {
  auto autodevice = (AutoDevice*)handle;
  try {
    auto transactionId = string(transaction_id);
    auto cashierId = string(cashier_id);
    PaymentMetadata metaData;

    for (int i = 0; i < metadata_size; ++i) {
      auto key = string(metadata[i].key);
      auto value = string(metadata[i].value);

      metaData[key] = value;
    }

    autodevice->createPaymentToken(amount, transactionId, cashierId, metaData)
      .then([callback, data](string token) { callback(JETBEEP_NO_ERROR, token.c_str(), data); })
      .catchError([callback, data](exception_ptr) { callback(JETBEEP_ERROR_IO, NULL, data); });
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API jetbeep_error_t jetbeep_autodevice_cancel_payment(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->cancelPayment();
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}

JETBEEP_API bool jetbeep_autodevice_is_mobile_connected(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  return autodevice->isMobileConnected();
}

JETBEEP_API const char* jetbeep_autodevice_version(jetbeep_autodevice_handle_t handle) {
  const size_t MAX_STR_SIZE = 20;
  static thread_local char version_str[20];
  auto autodevice = (AutoDevice*)handle;
  auto version = autodevice->version();

  memset(version_str, 0, MAX_STR_SIZE);
  memcpy(version_str, version.c_str(), version.size());
  return version_str;
}

JETBEEP_API unsigned long jetbeep_autodevice_device_id(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  return autodevice->deviceId();
}

JETBEEP_API void* jetbeep_autodevice_get_opaque(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  return autodevice->opaque;
}
JETBEEP_API void jetbeep_autodevice_set_opaque(jetbeep_autodevice_handle_t handle, void* opaque) {
  auto autodevice = (AutoDevice*)handle;
  autodevice->opaque = opaque;
}

JETBEEP_API jetbeep_state_t jetbeep_autodevice_state(jetbeep_autodevice_handle_t handle) {
  auto autodevice = (AutoDevice*)handle;
  return (jetbeep_state_t)autodevice->state();
}

JETBEEP_API void jetbeep_autodevice_set_payment_error_callback(jetbeep_autodevice_handle_t handle,
                                                               jetbeep_autodevice_payment_error_cb callback,
                                                               void* data) {
  auto autodevice = (AutoDevice*)handle;
  autodevice->paymentErrorCallback = [callback, data](const PaymentError& error) {
    jetbeep_error_t error_code;
    switch (error) {
    case PaymentError::discarded:
      error_code = JETBEEP_ERROR_PAYMENT_DISCARDED;
      break;
    case PaymentError::invalidPin:
      error_code = JETBEEP_ERROR_PAYMENT_INVALID_PIN;
      break;
    case PaymentError::network:
      error_code = JETBEEP_ERROR_PAYMENT_NETWORK;
      break;
    case PaymentError::security:
      error_code = JETBEEP_ERROR_PAYMENT_SECURITY;
      break;
    case PaymentError::server:
      error_code = JETBEEP_ERROR_PAYMENT_SERVER;
      break;
    case PaymentError::timeout:
      error_code = JETBEEP_ERROR_PAYMENT_TIMEOUT;
      break;
    case PaymentError::unknown:
      error_code = JETBEEP_ERROR_PAYMENT_UNKNOWN;
      break;
    case PaymentError::withdrawal:
      error_code = JETBEEP_ERROR_PAYMENT_WITHDRAWAL;
      break;
    default:
      error_code = JETBEEP_ERROR_PAYMENT_UNKNOWN;
      break;
    }
    callback(error_code, data);
  };
}

JETBEEP_API void jetbeep_autodevice_set_state_callback(jetbeep_autodevice_handle_t handle,
                                                       jetbeep_autodevice_state_cb callback,
                                                       void* data) {
  auto autodevice = (AutoDevice*)handle;
  autodevice->stateCallback = [callback, data](AutoDeviceState state, exception_ptr) {
    callback((jetbeep_state_t)state, data);
  };
}

JETBEEP_API void jetbeep_autodevice_set_mobile_connected_callback(jetbeep_autodevice_handle_t handle,
                                                                  jetbeep_autodevice_mobile_connected_cb callback,
                                                                  void* data) {
  auto autodevice = (AutoDevice*)handle;
  autodevice->mobileCallback = [callback, data](const SerialMobileEvent& event) {
    switch (event) {
    case SerialMobileEvent::connected:
      callback(true, data);
      break;
    case SerialMobileEvent::disconnected:
      callback(false, data);
      break;
    }
  };
}