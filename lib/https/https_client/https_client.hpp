#ifndef HTTPS_CLIENT_HPP
#define HTTPS_CLIENT_HPP

#include "../../utils/platform.hpp"

#ifdef PLATFORM_WIN
  #define HTTP_CLIENT_WINHTTP
#elif defined(PLATFORM_OSX)
  #define HTTP_CLIENT_NSURLSESSION
#elif defined(PLATFORM_LINUX)
  #define HTTP_CLIENT_LIBCURL
#else
  #define HTTP_CLIENT_BOOST_BEAST
#endif

#include "../../io/iocontext.hpp"
#include "../../utils/logger.hpp"
#include "../../utils/promise.hpp"
#include "../../utils/version.hpp"
#include "../http_errors.hpp"
#include <atomic>
#include <map>
#include <string>
#include <thread>

#ifdef HTTP_CLIENT_LIBCURL
  #include <curl/curl.h>
#endif

#define HTTP_USER_AGENT ("JetBeep usb-library/" + Version::currentVersion())
#define DEFAULT_HTTPS_PORT 443
#define DEFAULT_TIMEOUT_MS (30 * 1000)
#define HTTP_VERSION 11

using namespace std;

namespace JetBeep {
  enum class RequestMethod { GET, POST, PATCH };

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
    int timeout = DEFAULT_TIMEOUT_MS;
    IOContext ioContext;
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

    #ifdef HTTP_CLIENT_LIBCURL
    CURL* m_curl = nullptr;
    #endif

    #ifdef HTTP_CLIENT_NSURLSESSION
    void *m_task;
    RequestOptions m_options;
    void reject(std::exception_ptr exception);
    void resolve(Response response);
    #endif
  };

} // namespace JetBeep::Https

#endif