#ifndef PACKAGES_SEARCH_HPP
#define PACKAGES_SEARCH_HPP

#include <vector>
#include <string>

enum class PackageType {
    BOOTLOADER_SD_FW,
    APP_FW,
    UNKNOWN
};

enum class PackageDFUStyle {
    SERIAL,
    BLE,
    UNKNOWN
};

struct PackageInfo {
    std::string name = "";
    std::string path = "";
    PackageType type = PackageType::UNKNOWN;
    PackageDFUStyle dfuStyle = PackageDFUStyle::UNKNOWN;
};

inline PackageDFUStyle stringToDFUStyle(std::string str) {
    return str == "serial" ? PackageDFUStyle::SERIAL : 
        (str == "ble" ? PackageDFUStyle::BLE : PackageDFUStyle::UNKNOWN);
}

std::vector<PackageInfo> findZipPackages();

#endif