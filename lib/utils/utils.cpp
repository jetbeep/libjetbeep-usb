#include "utils.hpp"
#include <algorithm>
#include "stdio.h"
#include "string.h"

using namespace std;
using namespace JetBeep;

std::vector<std::string> Utils::splitString(const std::string& str, const std::string& delimiter) {
  vector<string> return_value;
  auto start = 0U;
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
  char tag[6];

  parse_results = sscanf(fwStr.c_str(), "%2hhu.%2hhu.%2hhu-%s", &major, &minor, &patch, tag);

  if (parse_results == EOF || parse_results < 3)
    return 0;

  if (strncasecmp(tag, "alpha", 5) == 0) {
    tag_number = 0;
  } else if (strncasecmp(tag, "beta", 4) == 0) {
    tag_number = 2;
  } else if (strncasecmp(tag, "rc", 2) == 0) {
    tag_number = 4;
  } else {
    tag_number = 9;
  }

  res = major * 1000 + minor * 100 + patch * 10 + tag_number;

  return res;
}