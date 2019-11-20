#ifndef JETBEEP_DEVICE_ERRORS
#define JETBEEP_DEVICE_ERRORS

#include <exception>

namespace JetBeep {
  namespace HttpErrors {
    class RequestError : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "HTTP request error";
      }
    };
  } // namespace Errors
} // namespace JetBeep

#endif