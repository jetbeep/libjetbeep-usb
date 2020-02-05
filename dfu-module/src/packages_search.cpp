#include "packages_search.hpp"
#include <regex>
#include <boost/filesystem.hpp>
#include "libjetbeep.hpp"

using namespace boost::filesystem;
using namespace std;

static JetBeep::Logger l("pkg_srch");

vector<string> findZipPackages() {
  vector<string> results;
  path cwd = current_path();
  string part1Path;
  string part2Path;

  regex pkgNameRe("^.+_(serial|ble)_dfu_.+update_part_(\\d)\\.zip$");

  try {
    if (!is_directory(cwd)) {
      throw runtime_error("Unable to get current dir");
    }
    for (directory_entry& x : directory_iterator(cwd)) {
      if (!is_regular_file(x)) { continue; }
      smatch match;
      auto path = x.path();
      const string fileName = path.filename().string();
      
      if (std::regex_match(fileName, match, pkgNameRe) &&  match.size() == 3) {
          if (match[2] == "1") {
              part1Path = path.string();
          } else if (match[2] == "2") {
              part2Path = path.string();
          }
      }
    }
  }
  catch (const filesystem_error& e) {
    l.e() << e.what() << JetBeep::Logger::endl;
    throw runtime_error("Unable to find zip packages");
  }
  if (!part1Path.empty()) {
      results.push_back(part1Path);
  }
  if (!part2Path.empty()) {
      results.push_back(part2Path);
  }
  return results;
}