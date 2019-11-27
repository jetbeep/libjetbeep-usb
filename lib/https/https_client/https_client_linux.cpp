#include "./https_client.hpp"

#ifdef HTTP_CLIENT_LIBCURL

#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <string>
//#include "../../io/iocontext.hpp"

using namespace JetBeep;
using namespace std;

static bool isCurlInited = false;

Https::HttpsClient::HttpsClient() : m_log("https_client") {
  m_isCanceled.store(false);
  m_isPending.store(false);
  if (!isCurlInited) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    isCurlInited = true;
  }
};

Https::HttpsClient::~HttpsClient() {
  m_isCanceled.store(true);
  if (m_thread.joinable()) {
    m_thread.join();
  }
  m_isPending.store(false);
}

Promise<Https::Response> Https::HttpsClient::request(RequestOptions& options) {
  if (m_isPending.load() == true) {
    throw runtime_error("previous request is not completed");
  }
  m_isCanceled.store(false);
  m_isPending.store(true);
  m_pendingRequest = Promise<Response>();
  m_thread = thread(&Https::HttpsClient::doRequest, this, options);
  return m_pendingRequest;
};

void Https::HttpsClient::doRequest(RequestOptions options) {
  m_thread.detach(); //TODO IOContext
  //auto ioContext = IOContext::context;
  char errorBuffer[CURL_ERROR_SIZE];
  std::string receiveBuffer;
  CURL* curl;
  try {
    /*CURLcode*/ int code;
    CURLcode res;

    auto onDataReceived = [](char* data, size_t size, size_t nmemb, std::string* buffer) -> int {
      if (buffer == nullptr) {
        return 0;
      }

      buffer->append(data, size * nmemb);
      return size * nmemb;
    };

    curl = curl_easy_init();

    if (!curl) {
      throw runtime_error("Unable to initializa curl");
    }

    code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    if (code != CURLE_OK) {
      throw runtime_error("Failed to set error buffer");
    }
    string url = "https://" + options.host + options.path;
    code += curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    code += curl_easy_setopt(curl, CURLOPT_PORT, options.port);
    code += curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, options.timeout);
    code += curl_easy_setopt(curl, CURLOPT_USERAGENT, HTTP_USER_AGENT);
    
    if (!options.body.empty()) {
      // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, options.body);
      code += curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, options.body.size());
      code += curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, options.body.c_str());
    }
    
    switch (options.method) {
    case RequestMethod::GET:
      code += curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
      break;
    case RequestMethod::POST:
      code += curl_easy_setopt(curl, CURLOPT_POST, 1L);
      break;
    }

    if (code != CURLE_OK) {
      throw runtime_error("Unable to curl_easy_setopt for some options");
    }

    code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onDataReceived);
    if (code != CURLE_OK) {
      throw runtime_error("Failed to set error buffer");
    }

    // set pointer which will be 4th param in writer func
    code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receiveBuffer);
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
    //TODO fix Segmentation fault
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      throw runtime_error(string(errorBuffer));
    }

    long statusCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);

    Response response;
    response.body = receiveBuffer;
    response.statusCode = statusCode;
    response.isHttpError = Https::HttpsClient::isErrorStatusCode(response.statusCode);

    m_isPending.store(false);

    m_log.d() << "API response (" << response.statusCode << "): " << response.body << Logger::endl;

    curl_easy_cleanup(curl);

    m_pendingRequest.resolve(response); 

  //TODO
    /*ioContext.post( [&this, &m_pendingRequest, =response]{ 
      m_pendingRequest.resolve(response); 
    });*/

  } catch (std::exception const& e) {
    if (curl) {
      curl_easy_cleanup(curl);
    }
    m_log.e() << e.what() << Logger::endl;
    m_pendingRequest.reject(make_exception_ptr(HttpErrors::NetworkError()));
  }
}

#endif