#ifndef HTTPS_CLIENT_HPP
#define HTTPS_CLIENT_HPP

#include "../utils/logger.hpp"
#include "../utils/promise.hpp"
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
    map<string, string> headers;     // TODO implement
    map<string, string> queryParams; // TODO implement
    RequestContentType contentType = RequestContentType::JSON;
    int timeout = DEFAULT_TIMEOUT_MS; // not implemented https://stackoverflow.com/questions/56828654/timeout-for-boostbeast-sync-http-client
  } RequestOptions;

  typedef struct {
    int statusCode;
    string body;
    bool isHttpError;
  } Response;

  class HttpsClient {
  public:
    Promise<Response> request(RequestOptions& options);
    HttpsClient();
    ~HttpsClient();

  private:
    Promise<Response> m_pendingRequest;
    std::thread m_thread;
    std::atomic<bool> m_isCanceled;
    std::atomic<bool> m_isPending;
    Logger m_log;
    void doRequest(RequestOptions options);

    static bool isErrorStatusCode(int statusCode) {
      int major = (int)(statusCode / 100);
      return major == 4 || major == 5;
    }
  };

} // namespace JetBeep::Https

#endif