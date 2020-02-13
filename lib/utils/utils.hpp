#ifndef JETBEEP_SPLIT__H
#define JETBEEP_SPLIT__H

#include <string>
#include <vector>
#include "logger.hpp"

namespace JetBeep {
  class Utils {
  public:
    static std::vector<std::string> splitString(const std::string& str, const std::string& delimiter = " ");
    static std::string toLowerCase(const std::string& input);
    static uint32_t deviceFWVerToNumber(const std::string& fwStr);
    static bool caseInsensetiveEqual(const std::string& str1, const std::string& str2);
    static void replaceInTemplate(std::string& text, const std::string& placeholder, const std::string& value);
  private:
    static Logger m_log;
  };
} // namespace JetBeep

#endif