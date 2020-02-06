#ifndef _INC_EXT_ERROR_STRING_H
#define _INC_EXT_ERROR_STRING_H

#include <stdint.h>
#include <string>

#include <exception>

enum class NRF_DFU_EXT_ERROR : int {
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

std::string getExtendedErrorMsg(int);

namespace DFU {
  class ExtendedError : public std::exception {
  public:
    std::string extErrMsg;
    int errorCode;
    ExtendedError(int errorCode = 0) : errorCode(errorCode) {
      extErrMsg = getExtendedErrorMsg(errorCode);
    }
    virtual char const* what() const noexcept {
      return extErrMsg.c_str();
    }
  };
} // namespace DFU



#endif // _INC_EXT_ERROR_STRING_H
