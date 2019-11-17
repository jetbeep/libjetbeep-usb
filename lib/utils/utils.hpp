#ifndef JETBEEP_SPLIT__H
#define JETBEEP_SPLIT__H

#include <vector>
#include <string>

namespace JetBeep {
  class Utils {
  public:
    static std::vector<std::string> splitString(const std::string& str, const std::string& delimiter = " ");
    static std::string toLowerCase(const std::string& input);
  };  
}

#endif