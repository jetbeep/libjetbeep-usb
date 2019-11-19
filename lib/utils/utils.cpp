#include "utils.hpp"
#include <algorithm>

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