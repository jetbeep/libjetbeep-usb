#include "platform.hpp"
#include "utils.hpp"
#include <algorithm>
#include <regex>
#include "stdio.h"
#include "string.h"

#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace JetBeep;

Logger Utils::m_log = Logger("utils");

std::vector<std::string> Utils::splitString(const std::string& str, const std::string& delimiter) {
  vector<string> return_value;
  size_t start = 0;
  auto end = str.find(delimiter);

  while (end != string::npos) {
    auto delimitedString = str.substr(start, end - start);
    return_value.push_back(delimitedString);
    start = end + delimiter.length();
    end = str.find(delimiter, start);
  }

  auto delimitedString = str.substr(start, end);
  return_value.push_back(delimitedString);

  return return_value;
}

string Utils::toLowerCase(const string& input) {
  string result = input;

  transform(result.begin(), result.end(), result.begin(), (int (*)(int))std::tolower);

  return result;
}

uint32_t Utils::deviceFWVerToNumber(const std::string& fwStr) {
  uint32_t res;
  int parse_results = 0;
  uint8_t major = 0, minor = 0, patch = 0, tag_number = 0;
  string tag;

  try {
    regex re("^(\\d{1,3}).(\\d{1,3}).(\\d{1,3})(?:[-]{1}(.{1,6})){0,1}$");
    smatch match;

    if (!regex_search(fwStr, match, re)) {
      m_log.e() << "unable to match regular expression" << Logger::endl;
      return 0;
    }
    if (match.size() != 5) {
      m_log.e() << "invalid size of matched items: " << match.size() << Logger::endl;
      return 0;
    }
    major = (uint8_t)stoi(match[1].str());
    minor = (uint8_t)stoi(match[2].str());
    patch = (uint8_t)stoi(match[3].str());
    tag = match[4].str();
  } catch (...) {
    m_log.e() << "unhandled exception" << Logger::endl;
    return 0;
  }

  if (Utils::caseInsensetiveEqual(tag, "alpha")) {
    tag_number = 0;
  } else if (Utils::caseInsensetiveEqual(tag, "beta")) {
    tag_number = 2;
  } else if (Utils::caseInsensetiveEqual(tag, "rc")) {
    tag_number = 4;
  } else {
    tag_number = 9;
  }

  res = major * 1000 + minor * 100 + patch * 10 + tag_number;

  return res;
}

bool Utils::caseInsensetiveEqual(const std::string& str1, const std::string& str2) {
  if (str1.size() != str2.size()) {
    return false;
  }
  return Utils::toLowerCase(str1) == Utils::toLowerCase(str2);
}

void  Utils::replaceInTemplate(string& text, const string& placeholder, const string& value) {
  boost::replace_all<string>(text, "\"" + placeholder + "\"", value);
}