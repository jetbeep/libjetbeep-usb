#include "ext_error.h"
#include "ext_error_string.hpp"

enum class NRF_DFU_EXT_ERROR {
  NO_ERROR = 0x00,
  INVALID_ERROR_CODE = 0x01,
  WRONG_COMMAND_FORMAT = 0x02,
  UNKNOWN_COMMAND = 0x03,
  INIT_COMMAND_INVALID = 0x04,
  FW_VERSION_FAILURE = 0x05,
  HW_VERSION_FAILURE = 0x06,
  SD_VERSION_FAILURE = 0x07,
  SIGNATURE_MISSING = 0x08,
  WRONG_HASH_TYPE = 0x09,
  HASH_FAILED = 0x0A,
  WRONG_SIGNATURE_TYPE = 0x0B,
  VERIFICATION_FAILED = 0x0C,
  INSUFFICIENT_SPACE = 0x0D,
  FW_ALREADY_PRESENT = 0x0E // not documented
};

static uint8_t lastErrorCode = 0;

void set_ext_error_code(uint8_t code) {
  lastErrorCode = code;
}

std::string getExtendedErrorMsg() {
  int code = lastErrorCode;
  lastErrorCode = NRF_DFU_EXT_ERROR::NO_ERROR; //reset code
  switch (lastErrorCode) {
  case (int)NRF_DFU_EXT_ERROR::NO_ERROR:
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