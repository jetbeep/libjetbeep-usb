#define JETBEEP_API_EXPORTS
#include "libjetbeep.h"
#include "libjetbeep.hpp"

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
                                                                jetbeep_autodevice_barcode_result_cb callback) {
  auto autodevice = (AutoDevice*)handle;
  try {
    autodevice->requestBarcodes()
      .then([callback](vector<Barcode> barcodes) {
        jetbeep_barcode_t* barcodesT = (jetbeep_barcode_t*)malloc(sizeof(jetbeep_barcode_t) * barcodes.size());
        size_t i = 0;
        for (auto it = barcodes.begin(); it != barcodes.end(); ++it) {
          barcodesT[i].barcode = (*it).value.c_str();
          barcodesT[i].barcode_size = (*it).value.size();
          barcodesT[i].type = (int)(*it).type;
          ++i;
        }
        callback(JETBEEP_NO_ERROR, barcodesT, barcodes.size());
        free(barcodesT);
      })
      .catchError([callback](exception_ptr error) { callback(JETBEEP_ERROR_IO, nullptr, 0); });
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
                                                              size_t transaction_id_size,
                                                              jetbeep_autodevice_payment_result_cb callback,
                                                              const char* cashier_id,
                                                              size_t cashier_id_size,
                                                              const jetbeep_payment_metadata_t* metadata,
                                                              size_t metadata_size) {
  auto autodevice = (AutoDevice*)handle;
  try {
    auto transactionId = string(transaction_id, transaction_id_size);
    auto cashierId = string(cashier_id, cashier_id_size);
    PaymentMetadata metaData;

    for (int i = 0; i < metadata_size; ++i) {
      auto key = string(metadata[i].key, metadata[i].key_size);
      auto value = string(metadata[i].value, metadata[i].value_size);

      metaData[key] = value;
    }

    autodevice->createPayment(amount, transactionId, cashierId, metaData)
      .then([callback] { callback(JETBEEP_NO_ERROR); })
      .catchError([callback](exception_ptr) { callback(JETBEEP_ERROR_IO); });
  } catch (const Errors::InvalidState&) {
    return JETBEEP_ERROR_INVALID_STATE;
  } catch (...) {
    return JETBEEP_ERROR_IO;
  }
  return JETBEEP_NO_ERROR;
}