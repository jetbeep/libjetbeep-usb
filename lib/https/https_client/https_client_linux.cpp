#include "./https_client.hpp"

#ifdef HTTP_CLIENT_LIBCURL

#include "../../io/iocontext_impl.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace JetBeep;
using namespace std;

typedef size_t(*CURL_WRITEFUNCTION_PTR)(void*, size_t, size_t,  std::string*);

static bool isCurlInited = false;

HttpsClient::HttpsClient() : m_log("https_client") {
  m_isCanceled.store(false);
  m_isPending.store(false);
  if (!isCurlInited) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    m_curl = curl_easy_init();
    if (!m_curl) {
      throw runtime_error("Unable to initializa curl");
    }
    isCurlInited = true;
  }
};

HttpsClient::~HttpsClient() {
  m_isCanceled.store(true);
  if (m_thread.joinable()) {
    m_thread.join();
  }
  m_isPending.store(false);
  curl_easy_cleanup(m_curl);
}

Promise<Response> HttpsClient::request(RequestOptions& options) {
  if (m_isPending.load() == true) {
    throw runtime_error("previous request is not completed");
  }
  m_isCanceled.store(false);
  m_isPending.store(true);
  m_pendingRequest = Promise<Response>();
  if (m_thread.joinable()) {
    m_thread.join();
  }
  m_thread = thread(&HttpsClient::doRequest, this, options);
  return m_pendingRequest;
};

void HttpsClient::doRequest(RequestOptions options) {
  char errorBuffer[CURL_ERROR_SIZE];
  std::string receiveBuffer;
  
  try {
    /*CURLcode*/ int code;
    CURLcode res;

    struct curl_slist *headersList = nullptr;

    auto onDataReceived = [](void* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
      data->append(static_cast<char*>(ptr), size * nmemb);
      return size * nmemb;
    };

    code = curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, errorBuffer);
    if (code != CURLE_OK) {
      throw runtime_error("Failed to set error buffer");
    }
    string url = "https://" + options.host + options.path;
    code += curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    code += curl_easy_setopt(m_curl, CURLOPT_PORT, options.port);
    code += curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, options.timeout);

    code += curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, NULL); //reseting

    string uaStr = HTTP_USER_AGENT;
    code += curl_easy_setopt(m_curl, CURLOPT_USERAGENT, uaStr.c_str());

    m_log.d() << "https request to " << url << " " + (options.method == RequestMethod::GET ? string("(GET)") : string("(POST)")) << Logger::endl;
    
    if (!options.body.empty()) {
      switch (options.contentType) {
      case RequestContentType::JSON: {
        headersList = curl_slist_append(headersList, "Content-Type: application/json");
        headersList = curl_slist_append(headersList, "Accept: application/json");
        break;
      }
      default:
        throw runtime_error("Content type not supported");
      }
      
      curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headersList);

      m_log.d() << "---- request data -----" << Logger::endl;
      m_log.d() << options.body << Logger::endl << Logger::endl;
      
      code += curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, options.body.size());
      //code += curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, options.body.c_str());
      code += curl_easy_setopt(m_curl, CURLOPT_COPYPOSTFIELDS, options.body.c_str());
    }
    
    switch (options.method) {
    case RequestMethod::GET:
      //code += curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1L);
      code +=  curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "GET"); //used to hack: GET + body
      break;
    case RequestMethod::POST:
      code += curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
      break;
    case RequestMethod::PATCH:
      code +=  curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "PATCH");
      break;
    default:
      throw runtime_error("HTTP method not supported");
    }

    if (code != CURLE_OK) {
      throw runtime_error("Unable to curl_easy_setopt for some options");
    }

    code = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, static_cast<CURL_WRITEFUNCTION_PTR>(onDataReceived));
    if (code != CURLE_OK) {
      throw runtime_error("Failed to set error buffer");
    }

    // set pointer which will be 4th param in writer func
    code = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &receiveBuffer);
    if (code != CURLE_OK) {
      throw runtime_error("Failed to set error buffer");
    }

#ifdef SKIP_PEER_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
    
    /* Perform the request */
    res = curl_easy_perform(m_curl);
    if (res != CURLE_OK) {
      throw runtime_error(string(errorBuffer));
    }

    long statusCode;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &statusCode);

    Response response;
    response.body = receiveBuffer;
    response.statusCode = statusCode;
    response.isHttpError = HttpsClient::isErrorStatusCode(response.statusCode);

    m_isPending.store(false);

    m_log.d() << "API response (" << response.statusCode << "): " << response.body << Logger::endl;

    options.ioContext.m_impl->ioService.post([this, response]{
      m_pendingRequest.resolve(response); 
    });
  } catch (std::exception const& e) {
    m_log.e() << e.what() << Logger::endl;
    options.ioContext.m_impl->ioService.post([this]{
      m_pendingRequest.reject(make_exception_ptr(HttpErrors::NetworkError()));
    });
  }
}

#endif