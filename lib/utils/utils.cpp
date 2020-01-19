#include "platform.hpp"
#include "utils.hpp"
#include <algorithm>
#include "stdio.h"
#include "string.h"

using namespace std;
using namespace JetBeep;

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
  const int maxTagSize = 6;
  char cTag[maxTagSize];
  string tag;

  parse_results = sscanf_s(fwStr.c_str(), "%2hhu.%2hhu.%2hhu-%s", &major, &minor, &patch, cTag, maxTagSize);

  if (parse_results == EOF || parse_results < 3) {
    return 0;
  }
  tag.assign(cTag, strlen(cTag));

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