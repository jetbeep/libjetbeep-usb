#ifndef JETBEEP_SPLIT__H
#define JETBEEP_SPLIT__H

#include <vector>
#include <string>

namespace JetBeep {
  std::vector<std::string> splitString(const std::string& str, const std::string& delimiter = " ");
}

#endif