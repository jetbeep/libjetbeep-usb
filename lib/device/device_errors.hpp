#ifndef JETBEEP_DEVICE_ERRORS
#define JETBEEP_DEVICE_ERRORS

#include <exception>

namespace JetBeep {
  namespace Errors {
    class DeviceNotOpened : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Device is not opened";
      }
    };

    class OperationInProgress : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Another command is being executed now";
      }
    };

    class OperationTimeout : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Operation timed out";
      }
    };

    class InvalidResponse : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Device returned invalid response";
      }
    };

    class InvalidResponseWithReason : public std::exception {
    public:
      InvalidResponseWithReason(std::string code) :m_reason("Device returned error response: " + code), m_code(code) { }
      virtual char const* what() const noexcept {
        return m_reason.c_str();
      }
      std::string getErrorCode() {
        return m_code;
      }

    protected:
      std::string m_reason;
      std::string m_code;
    };

    class DeviceLost : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Device returned invalid response";
      }
    };

    class InvalidState : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Invalid state";
      }
    };

    class ProtocolError : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Protocol error";
      }
    };

    class IOError : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Input-output error";
      }
    };

    class OperationCancelled : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Operation cancelled";
      }
    };

    class FirmwareVersionNotSupported : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Device firmware version is not supported";
      }
    };

    class NullPointerError : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Null pointer encounter";
      }
    };
  } // namespace Errors
} // namespace JetBeep

#endif