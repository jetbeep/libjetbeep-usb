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

  typedef struct {
    string chipId;
    string fwVersion; //the only field that can be actualy changed in dfu module
  } DeviceConfigUpdateRequest;

  string deviceConfigUpdateToJSON(DeviceConfigUpdateRequest& data);

} // namespace JetBeep::PortalAPI

#endif