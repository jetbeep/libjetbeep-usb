#ifndef JETBEEP_HTTP_ERRORS
#define JETBEEP_HTTP_ERRORS

#include <exception>

namespace JetBeep {
  namespace HttpErrors {
    class RequestError : public std::exception {
      public:
      RequestError(std::string msg = "HTTP request error"): m_serverMessage(msg){}
      std::string getRequestError() {
        return m_serverMessage;
      }
      virtual char const* what() const noexcept {
        return m_serverMessage.c_str();
      }
      private:
      std::string m_serverMessage;
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
    class APIError : public std::exception {
    public:
      virtual char const* what() const noexcept {
        return "API error";
      }
    };
  } // namespace HttpErrors
} // namespace JetBeep

#endif