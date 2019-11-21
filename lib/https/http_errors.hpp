#ifndef JETBEEP_HTTP_ERRORS
#define JETBEEP_HTTP_ERRORS

#include <exception>

namespace JetBeep {
  namespace HttpErrors {
    class RequestError : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "HTTP request error";
      }
    };
    class NetworkError : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "Network error";
      }
    };
    class ServerError : public std::exception {
    public:
      ServerError(int statusCode = 0) : statusCode(statusCode) {
      }
      virtual char const* what() const noexcept {
        return "Server error";
      }
      int statusCode;
    };

  } // namespace HttpErrors
} // namespace JetBeep

#endif