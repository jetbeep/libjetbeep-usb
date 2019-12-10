#include "./https_client.hpp"

#ifdef HTTP_CLIENT_WINHTTP
#include "../../io/iocontext_impl.hpp"

#include <windows.h>
#include <winhttp.h>
#include <schannel.h>

#ifndef WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
#define WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY 4 // due to lack in mingw 8.1.0
#endif

#include <cstdlib>
#include <iostream>
#include <string>

using namespace JetBeep;
using namespace std;

HttpsClient::HttpsClient() : m_log("https_client") {
  m_isCanceled.store(false);
  m_isPending.store(false);
};

HttpsClient::~HttpsClient() {
  m_isCanceled.store(true);
  if (m_thread.joinable()) {
    m_thread.join();
  }
  m_isPending.store(false);
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
  try {
    /* */
    DWORD dwSize = 0;
    BOOL bResults = false;
    HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;
    auto handleSystemErrors = []() {
      DWORD dw = GetLastError();
      throw runtime_error("winHTTP error code: " + std::to_string(dw)); // WINHTTP_ERROR_BASE + code
    };

    // Use WinHttpOpen to obtain a session handle.
    string uaStr = HTTP_USER_AGENT;
    std::wstring stemp = std::wstring(uaStr.begin(), uaStr.end()); // only ASCII or ISO-8859-1
    LPCWSTR swUAStr = stemp.c_str();
    hSession = WinHttpOpen(swUAStr, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession) {
      handleSystemErrors();
    }

    DWORD timeout = (DWORD)options.timeout;

    bResults = WinHttpSetOption(hSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(DWORD));
    if (!bResults) {
      handleSystemErrors();
    }

    std::wstring hostWStr = std::wstring(options.host.begin(), options.host.end());
    hConnect = WinHttpConnect(hSession, hostWStr.c_str(), options.port, 0);
    if (!hConnect) {
      handleSystemErrors();
    }

    // Create an HTTP request handle.
    std::wstring method;
    switch (options.method) {
    case RequestMethod::GET:
      method = L"GET";
      break;
    case RequestMethod::POST:
      method = L"POST";
      break;
    default:
      throw runtime_error("HTTP method not supported");
    }
    std::wstring wsJsonMIME = L"application/json";
    LPCWSTR acceptTypes[] = {wsJsonMIME.c_str(), NULL};
    std::wstring wsPath = std::wstring(options.path.begin(), options.path.end());
    hRequest = WinHttpOpenRequest(hConnect, method.c_str(), wsPath.c_str(), L"HTTP/1.1", WINHTTP_NO_REFERER,
                                  acceptTypes, WINHTTP_FLAG_REFRESH | WINHTTP_FLAG_SECURE);
    if (!hRequest) {
      handleSystemErrors();
    }

    // add headers
    bResults = WinHttpAddRequestHeaders(hRequest, L"Content-Type: application/json\r\n", (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
    if (!bResults) {
      handleSystemErrors();
    }

    std::wstring contentLengthHeader = L"Content-Length: ";
    contentLengthHeader += std::to_wstring(options.body.length()); // last header without CR/LF

    bResults = WinHttpAddRequestHeaders(hRequest, contentLengthHeader.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_REPLACE);
    if (!bResults) {
      handleSystemErrors();
    }

    // Send a request.
    LPVOID requestPayload = options.body.length() ? (LPVOID)options.body.c_str() : WINHTTP_NO_REQUEST_DATA;
    bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, requestPayload, options.body.length(),
                                  WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, 0);
    if (!bResults) {
      handleSystemErrors();
    }

    // End the request.
    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults) {
      handleSystemErrors();
    } 

    // Keep checking for data until there is nothing left.
    stringstream result;
    do {
      // Check for available data.
      dwSize = 0;
      if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
        m_log.e() << "WinHttpQueryDataAvailable error" << Logger::endl;
        handleSystemErrors();
      }

      if (dwSize <= 0) {
        break;
      }

      string receiveData = string();
      receiveData.resize(dwSize);

      auto cBuf = receiveData.c_str();

      DWORD bytesRecvd = 0;

      if (!WinHttpReadData(hRequest, (LPVOID)cBuf, dwSize, &bytesRecvd)) {
        m_log.e() << "WinHttpReadData error" << Logger::endl;
        handleSystemErrors();
      }
      result << receiveData;

    } while (dwSize > 0 && !m_isCanceled.load());

    DWORD dwStatusCode = 0;
    DWORD dwStatusCodeSize = sizeof(dwStatusCode);

    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
                        &dwStatusCode, &dwStatusCodeSize, WINHTTP_NO_HEADER_INDEX);

    // Close any open handles.
    if (hRequest) {
      WinHttpCloseHandle(hRequest);
    }
    if (hConnect) {
      WinHttpCloseHandle(hConnect);
    }
    if (hSession) {
      WinHttpCloseHandle(hSession);
    }

    Response response;
    response.body = result.str();
    response.statusCode = dwStatusCode;
    response.isHttpError = HttpsClient::isErrorStatusCode(response.statusCode);

    m_isPending.store(false);

    m_log.d() << "API response (" << response.statusCode << "): " << response.body << Logger::endl;

    options.ioContext.m_impl->ioService.post([this, response] { m_pendingRequest.resolve(response); });
  } catch (std::exception const& e) {
    m_log.e() << e.what() << Logger::endl;
    options.ioContext.m_impl->ioService.post(
      [this] { m_pendingRequest.reject(make_exception_ptr(HttpErrors::NetworkError())); });
  }
}

#endif