#ifndef HTTPS_CLIENT_HPP
#define HTTPS_CLIENT_HPP

#include "../utils/promise.hpp"
#include <string>

namespace JetBeep {
  class HttpsClient {
      Promise<std::string> request();

      private:
      Promise<std::string> m_pending_request;
  };

} // namespace JetBeep

#endif