#ifndef PORTAL_REQUEST
#define PORTAL_REQUEST

#include <string>
#include <vector>

#include "../device/device_types.hpp"

using namespace std;

namespace JetBeep::PortalAPI {
  typedef struct {
    string chipId;
  } DeviceConfigRequest;

} // namespace JetBeep::PortalAPI

#endif