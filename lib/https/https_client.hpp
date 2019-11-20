#ifndef HTTPS_CLIENT_HPP
#define HTTPS_CLIENT_HPP

#include "../utils/promise.hpp"
#include "../utils/logger.hpp"
#include "./http_errors.hpp"
#include <atomic>
#include <map>
#include <string>
#include <thread>

#define DEFAULT_HTTPS_PORT 443
#define DEFAULT_TIMEOUT_MS (30 * 1000)
#define HTTP_VERSION 11

using namespace std;

namespace JetBeep::Https {
  enum class RequestMethod { GET, POST };

  enum class RequestContentType { JSON };

  typedef struct {
    RequestMethod method;
    string body;
    string host;
    int port = DEFAULT_HTTPS_PORT;
    string path;
    map<string, string> headers; //TODO implement
    map<string, string> queryParams; //TODO implement
    RequestContentType contentType = RequestContentType::JSON;
    int timeout = DEFAULT_TIMEOUT_MS; //not implemented https://stackoverflow.com/questions/56828654/timeout-for-boostbeast-sync-http-client
  } RequestOptions;

  class HttpsClient {
  public:
    Promise<std::string> request(RequestOptions& options);
    HttpsClient();
    ~HttpsClient();

  private:
    Promise<std::string> m_pendingRequest;
    std::thread m_thread;
    std::atomic<bool> m_isCanceled;
    std::atomic<bool> m_isPending;
    Logger m_log;
    void doRequest(RequestOptions options);
  };

} // namespace JetBeep::Https

#endif