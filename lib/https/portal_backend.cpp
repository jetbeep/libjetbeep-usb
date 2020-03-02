#include "../utils/platform.hpp"
#include "./portal_backend.hpp"
#include "../utils/logger.hpp"
#include "./https_client/https_client.hpp"
#include "../io/iocontext_impl.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <time.h>

#define BASE_API_PATH "/api/v0.9/portal/devices/public/"

using namespace JetBeep;
using namespace std;

class PortalBackend::Impl {
public:
  Impl(string serverHost, IOContext context, int port = 443)
    : m_serverHost(serverHost), m_port(port), m_log("portal"), m_context(context){};

  ~Impl();

  Promise<DeviceConfigResponse> getDeviceConfig(DeviceConfigRequest &requestData);
  Promise<void> updateDeviceConfig(DeviceConfigUpdateRequest &requestData);

private:
  IOContext m_context;
  HttpsClient m_httpsClient;
  string m_serverHost;
  int m_port;
  Logger m_log;

  RequestOptions getRequestOptions(string path, RequestMethod method, RequestContentType contentType = RequestContentType::JSON);
};

PortalBackend::Impl::~Impl() = default;

PortalBackend::PortalBackend(PortalHostEnv env, IOContext context)
  : m_impl(new Impl(env == PortalHostEnv::Production ? "prod.jetbeep.com" : "dev.jetbeep.com", context)) {
}

PortalBackend::~PortalBackend() = default;

Promise<DeviceConfigResponse> PortalBackend::getDeviceConfig(DeviceConfigRequest &requestData) {
  return m_impl->getDeviceConfig(requestData);
}

Promise<void> PortalBackend::updateDeviceConfig(DeviceConfigUpdateRequest &requestData) {
  return m_impl->updateDeviceConfig(requestData);
}

RequestOptions PortalBackend::Impl::getRequestOptions(string path, RequestMethod method, RequestContentType contentType) {
  RequestOptions options;
  options.method = method;
  options.contentType = contentType;
  options.host = m_serverHost;
  options.port = m_port;
  options.path = path;
  options.ioContext = m_context;
  return options;
}

Promise<DeviceConfigResponse> PortalBackend::Impl::getDeviceConfig(DeviceConfigRequest &requestData) {
  const string path = BASE_API_PATH + requestData.chipId + "/config";

  auto options = getRequestOptions(path, RequestMethod::GET);

  return m_httpsClient.request(options).thenPromise<DeviceConfigResponse, Promise>([&](Response res) {
    auto promise = Promise<DeviceConfigResponse>();
    if (res.statusCode == 404) {
      promise.reject(make_exception_ptr(HttpErrors::RequestError("Server response: device not found")));
      return promise;
    }
    if (res.isHttpError) {
      promise.reject(make_exception_ptr(HttpErrors::ServerError(res.statusCode)));
      return promise;
    }
    try {
      DeviceConfigResponse result;
      result.config = parseDeviceConfigResult(res.body);
      result._rawResponse = res.body;
      result.statusCode = res.statusCode;
      promise.resolve(result);
    } catch (...) {
      promise.reject(make_exception_ptr(HttpErrors::APIError()));
    }
    return promise;
  });
}

Promise<void> PortalBackend::Impl::updateDeviceConfig(DeviceConfigUpdateRequest &requestData) {
  const string path = BASE_API_PATH + requestData.chipId + "/config";

  auto options = getRequestOptions(path, RequestMethod::PATCH);
  options.body = deviceConfigUpdateToJSON(requestData);

  return m_httpsClient.request(options).thenPromise([&](Response res) {
    auto promise = Promise<void>();
    if (res.statusCode == 404) {
      promise.reject(make_exception_ptr(HttpErrors::RequestError("device not found")));
      return promise;
    }
    if (res.isHttpError) {
      promise.reject(make_exception_ptr(HttpErrors::ServerError(res.statusCode)));
      return promise;
    }
    promise.resolve();
    return promise;
  });
}