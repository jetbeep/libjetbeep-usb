#include "packages_search.hpp"
#include <regex>
#include <boost/filesystem.hpp>
#include "libjetbeep.hpp"
#include <algorithm>

using namespace boost::filesystem;
using namespace std;

static JetBeep::Logger l("pkg_srch");

vector<PackageInfo> findZipPackages() {
  vector<PackageInfo> results;
  path cwd = current_path();

  regex pkgNameRe("^.+_(serial|ble)_dfu_.+update_part_(\\d)\\.zip$");

  try {
    if (!is_directory(cwd)) {
      throw runtime_error("Unable to get current dir");
    }
    for (directory_entry& x : directory_iterator(cwd)) {
      if (!is_regular_file(x)) {
        continue;
      }
      smatch match;
      auto path = x.path();
      const string fileName = path.filename().string();

      if (std::regex_match(fileName, match, pkgNameRe) && match.size() == 3) {
        PackageInfo pkg;
        pkg.name = fileName;
        pkg.path = path.string();
        pkg.dfuStyle = stringToDFUStyle(match[1]);
        if (match[2] == "1") {
          pkg.type = PackageType::BOOTLOADER_SD_FW;
          results.push_back(pkg);
        } else if (match[2] == "2") {
          pkg.type = PackageType::APP_FW;
          results.push_back(pkg);
        }
      }
    }
  } catch (const filesystem_error& e) {
    l.e() << e.what() << JetBeep::Logger::endl;
    throw runtime_error("Unable to find zip packages");
  }
  sort(results.begin(), results.end(), [](PackageInfo p1, PackageInfo p2) -> bool {
    return p1.type == PackageType::BOOTLOADER_SD_FW; //make sure bl_sd package is the first
  });
  return results;
}