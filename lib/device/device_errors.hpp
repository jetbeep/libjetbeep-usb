#ifndef JETBEEP_DEVICE_ERRORS
#define JETBEEP_DEVICE_ERRORS

#include <exception>

namespace JetBeep {
  namespace Errors {
      class DeviceNotOpened: public std::exception {
      public:
        virtual char const* what() const noexcept { return "Device is not opened"; } 
      };

      class OperationInProgress: public std::exception {
      public:
        virtual char const* what() const noexcept { return "Another command is being executed now"; } 
      };

      class OperationTimeout: public std::exception {
      public:
        virtual char const* what() const noexcept { return "Operation timed out"; } 
      };

      class InvalidResponse: public std::exception {
      public:
        virtual char const* what() const noexcept { return "Device returned invalid response"; } 
      };
  }
}

#endif