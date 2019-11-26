#include "version.hpp"

using namespace JetBeep;
using namespace std;

string Version::currentVersion() {
  return string(JETBEEP_VERSION); // see CMakeLists.txt and VERSION files in root directory
}