#include "ext_error.h"
#include "ext_error_string.hpp"

static int lastErrorCode = 0;

void set_ext_error_code(int code) {
  lastErrorCode = code;
}

int get_ext_error_code() {
  int code = lastErrorCode;
  lastErrorCode = (int)NRF_DFU_EXT_ERROR::NO_ERROR_CODE; // reset code
  return code;
}

std::string getExtendedErrorMsg(int code) {
  switch (code) {
  case (int)NRF_DFU_EXT_ERROR::NO_ERROR_CODE:
    return "An error happened, but its extended error code hasn't been set.";
  case (int)NRF_DFU_EXT_ERROR::INVALID_ERROR_CODE:
    return "An error happened, but its extended error code is incorrect.";
  case (int)NRF_DFU_EXT_ERROR::WRONG_COMMAND_FORMAT:
    return "The format of the command was incorrect.";
  case (int)NRF_DFU_EXT_ERROR::UNKNOWN_COMMAND:
    return "Command successfully parsed, but it is not supported or unknown.";
  case (int)NRF_DFU_EXT_ERROR::INIT_COMMAND_INVALID:
    return "The init command is invalid. The init packet either has an invalid update type or it is missing required "
           "fields for the update type (for example, the init packet for a SoftDevice update is missing the SoftDevice "
           "size field).";
  case (int)NRF_DFU_EXT_ERROR::FW_VERSION_FAILURE:
    return "The firmware version is too low. For an application, the version must be greater than the current "
           "application. For a bootloader, it must be greater than or equal to the current version. This requirement "
           "prevents downgrade attacks.";
  case (int)NRF_DFU_EXT_ERROR::HW_VERSION_FAILURE:
    return "The hardware version of the device does not match the required hardware version for the update.";
  case (int)NRF_DFU_EXT_ERROR::SD_VERSION_FAILURE:
    return "The array of supported SoftDevices for the update does not contain the FWID of the current SoftDevice.";
  case (int)NRF_DFU_EXT_ERROR::SIGNATURE_MISSING:
    return "The init packet does not contain a signature. This bootloader requires DFU updates to be signed.";
  case (int)NRF_DFU_EXT_ERROR::WRONG_HASH_TYPE:
    return "The hash type that is specified by the init packet is not supported by the DFU bootloader.";
  case (int)NRF_DFU_EXT_ERROR::HASH_FAILED:
    return "The hash of the firmware image cannot be calculated.";
  case (int)NRF_DFU_EXT_ERROR::WRONG_SIGNATURE_TYPE:
    return "The type of the signature is unknown or not supported by the DFU bootloader.";
  case (int)NRF_DFU_EXT_ERROR::VERIFICATION_FAILED:
    return "The hash of the received firmware image does not match the hash in the init packet.";
  case (int)NRF_DFU_EXT_ERROR::INSUFFICIENT_SPACE:
    return "The available space on the device is insufficient to hold the firmware.";
  case (int)NRF_DFU_EXT_ERROR::FW_ALREADY_PRESENT:
    return "The requested firmware to update was already present on the system.";
  default:
    return "Unknown extended error code";
  }
}