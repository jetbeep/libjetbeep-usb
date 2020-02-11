#ifndef PORTAL_BACKEND_HPP
#define PORTAL_BACKEND_HPP

#include "../io/iocontext.hpp"
#include "../utils/promise.hpp"
#include "./portal_request.hpp"
#include "./portal_response.hpp"
#include "./http_errors.hpp"

#include <memory>

using namespace std;

namespace JetBeep {

  using namespace PortalAPI;

  enum class PortalHostEnv : int { Development = 0, Production = 1 };
  
  class PortalBackend {
  public:
    ~PortalBackend();
    PortalBackend(PortalHostEnv env, IOContext context = IOContext::context);

    Promise<DeviceConfigResponse> getDeviceConfig(DeviceConfigRequest &requestData);

    void* opaque;

  private:
    class Impl;
    unique_ptr<Impl> m_impl;
  };

} // namespace JetBeep

#endif