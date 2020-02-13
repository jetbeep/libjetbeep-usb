#include "./portal_request.hpp"
#include "../utils/utils.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

using namespace std;
using namespace JetBeep;

string PortalAPI::deviceConfigUpdateToJSON(DeviceConfigUpdateRequest& data) {
  pt::ptree json;
  json.put("fwVersion", data.fwVersion);

  std::stringstream stream;
  pt::write_json(stream, json);

  return stream.str();
}