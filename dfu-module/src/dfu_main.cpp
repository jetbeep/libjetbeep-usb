#include "../lib/utils/platform.hpp"
#include "dfu_main.hpp"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "uart_drv.h"
#include "delay_connect.h"
#include "uart_slip.h"
#include "dfu.h"
#include "dfu_serial.h"
#include "logging.h"
#include <future>
#include "libjetbeep.hpp"
#include "sync_serial_device.hpp"
#include "packages_search.hpp"
#include "ext_error_string.hpp"
#include "ext_error.h"
#include "device/device_utils.hpp"


using namespace std;
using namespace JetBeep;

Logger l("main");

static DeviceCandidate findJetBeepDeviceCandidate() {
  l.i() << "Waiting for a JetBeep device ..." << Logger::endl;
  std::promise<DeviceCandidate> detectionPromise;
  DeviceDetection deviceDetection;
  deviceDetection.callback = [&detectionPromise](const DeviceDetectionEvent& event, const DeviceCandidate& candidate) {
    if (event == DeviceDetectionEvent::added) {
      detectionPromise.set_value(candidate);
    } else {
      detectionPromise.set_exception(make_exception_ptr(std::runtime_error("device removed")));
    }
  };
  auto detectionReady = detectionPromise.get_future();
  deviceDetection.start();
  detectionReady.wait();
  deviceDetection.stop();
  const DeviceCandidate& candidate = detectionReady.get();
  l.i() << "Found JetBeep device:" << candidate.path << " vid: " << candidate.vid << " pid: " << candidate.pid << Logger::endl;
  delay_boot(); //to handle case when device is just connected but not ready yet (win) 
  return candidate;
}

static void resolveMcp2200Issue(JetBeep::SerialDevice& serial) {
  std::promise<void> issuePromise;
  auto readyFuture = issuePromise.get_future();

  serial.get(DeviceParameter::deviceId)
    .then([&issuePromise](string _deviceId) { issuePromise.set_value(); })
    .catchError([&issuePromise](const exception_ptr& ex) {
      l.v() << "resolveMcp2200Issue exception" << Logger::endl;
      issuePromise.set_value();
    });

  readyFuture.wait();
}

static bool isValidDeviceInfo(DeviceInfo &deviceInfo) {
  return deviceInfo.bootState == DeviceBootState::APP && deviceInfo.chipId.length() == 16;
}

//on 52840 path may change after each device reboot
static string updateDeviceSystemPath(DeviceInfo &deviceInfo) {
  return deviceInfo.systemPath = findJetBeepDeviceCandidate().path;
}

static DeviceInfo getDeviceInfo(bool updateSystemPath = false) {
  l.v() << "getDeviceInfo call" << Logger::endl;
  static string systemPath;
  DeviceInfo deviceInfo;
  if (updateSystemPath || systemPath.empty()) {
    systemPath = updateDeviceSystemPath(deviceInfo);
  } else {
    deviceInfo.systemPath = systemPath;
  }
  
  JetBeep::SerialDevice serial;
  serial.open(deviceInfo.systemPath);
  resolveMcp2200Issue(serial);
  std::promise<bool> infoReadPromise;
  auto infoReady = infoReadPromise.get_future();

  serial.get(DeviceParameter::deviceId)
    .thenPromise<string, Promise>([&infoReadPromise, &deviceInfo, &serial](string _deviceId) {
      try {
        uint32_t deviceId = stoul(_deviceId, nullptr, 16);
        deviceInfo.deviceId = deviceId;
      } catch (...) {
        infoReadPromise.set_exception(make_exception_ptr(runtime_error("Unable to read deviceId")));
      }
      return serial.get(DeviceParameter::version);
    })
    .thenPromise<string, Promise>([&infoReadPromise, &deviceInfo, &serial](string version) {
      deviceInfo.version = version;
      return serial.get(DeviceParameter::chipId);
    })
    .then([&infoReadPromise, &deviceInfo](string chipId) { 
      deviceInfo.chipId = chipId; 
      infoReadPromise.set_value(true);
    })
    .catchError([&infoReadPromise](const exception_ptr& ex) { 
      l.d() << "getDeviceInfo exception" << Logger::endl;
      infoReadPromise.set_exception(ex); 
    });

  infoReady.wait();
  try {
    if (infoReady.get()) {
      deviceInfo.bootState = DeviceBootState::APP;
    }
  } catch (const exception& ex) {
    l.w() << ex.what() << Logger::endl;
    deviceInfo.bootState = DeviceBootState::UNKNOWN;
  }
  return deviceInfo;
}

static void updateFirmwareProcedure(DeviceInfo& deviceInfo, vector<PackageInfo>& zipPackages) {
  int err_code = 0;
  Logger dfuLogger("dfu");
  logger_set_backend(&dfuLogger);

  DFU::SyncSerialDevice syncSerialDevice = DFU::SyncSerialDevice();
  uart_drv_t uart_drv = {&syncSerialDevice};

  syncSerialDevice.open(deviceInfo.systemPath);

  if (deviceInfo.bootState == DeviceBootState::APP) {
    syncSerialDevice.enterDFUMode();
    syncSerialDevice.close();
    delay_boot();
    syncSerialDevice.open(deviceInfo.systemPath);
  }

  l.i() << "Starting firmware update procedure." << Logger::endl;

  auto onError = []() { throw runtime_error("Unable to complete firmware update procedure."); };

  int pkgInx = 0;
  for (PackageInfo pkg : zipPackages) {
    if (pkgInx > 0) {
      //52840 may change port after reboot
      syncSerialDevice.close();
      delay_boot();
      updateDeviceSystemPath(deviceInfo);
      syncSerialDevice.open(deviceInfo.systemPath);
    }
    // test that device in bootloader mode
    err_code = dfu_serial_ping(&uart_drv, 0x04 /* any value */);
    if (err_code == 0) {
      deviceInfo.bootState = DeviceBootState::BOOTLOADER;
    } else {
      onError();
    }

    l.d() << "Processing firmware package: " << pkg.name << Logger::endl;

    dfu_param_t dfu_param;
    dfu_param.p_uart = &uart_drv;
    dfu_param.p_pkg_file = (char*)pkg.path.c_str();
    err_code = dfu_send_package(&dfu_param);

    if (err_code != 0) {
      int extErrorCode = get_ext_error_code();
      if (pkg.type == PackageType::BOOTLOADER_SD_FW && zipPackages.size() != 1 &&
          extErrorCode == (int)NRF_DFU_EXT_ERROR::FW_VERSION_FAILURE) {
        err_code = 0; // continue to app update assuming that bootloader and soft device are up to date already
        l.i() << "Bootloader and SD are up to date" << Logger::endl;
      } else if (extErrorCode != (int)NRF_DFU_EXT_ERROR::NO_ERROR_CODE) {
        throw DFU::ExtendedError(extErrorCode);
      } else {
        onError();
      }
    }
    pkgInx++;
  }
}

static DeviceConfig getDevicePortalConfig(PortalBackend& backend, DeviceInfo& deviceInfo) {
  std::promise<DeviceConfig> reqPromise;
  auto reqFuture = reqPromise.get_future();
  DeviceConfigRequest request;
  request.chipId = deviceInfo.chipId;

  backend.getDeviceConfig(request)
    .then([&reqPromise](DeviceConfigResponse res) { reqPromise.set_value(res.config); })
    .catchError([&reqPromise](const exception_ptr& ex) { reqPromise.set_exception(ex); });

  reqFuture.wait();
  return reqFuture.get();
}

static void updateDevicePortalConfig(PortalBackend& backend, DeviceInfo& deviceInfo) {
  std::promise<void> reqPromise;
  auto reqFuture = reqPromise.get_future();
  DeviceConfigUpdateRequest request;
  request.chipId = deviceInfo.chipId;
  request.fwVersion = deviceInfo.version;

  backend.updateDeviceConfig(request)
    .then([&reqPromise]() { reqPromise.set_value(); })
    .catchError([&reqPromise](const exception_ptr& ex) {
      reqPromise.set_exception(ex);
    });

  reqFuture.wait();
  return reqFuture.get();
}

static void writeDeviceConfig(DeviceConfig& config, JetBeep::SerialDevice& serial) {
  std::promise<void> writePromise;
  auto writeDone = writePromise.get_future();

  SerialBeginPrivateMode configMode = config.signatureType == "setup" ? SerialBeginPrivateMode::setup : SerialBeginPrivateMode::config;

  serial.beginPrivate(SerialBeginPrivateMode::config)
    .thenPromise([&]() {
      return serial.set(DeviceParameter::shopId,  Utils::numberToHexString(config.shopId));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::shopKey, config.shopKey);
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::domainShopId,  Utils::numberToHexString(config.domainShopId));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::merchantId,  Utils::numberToHexString(config.merchantId));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::cashierId,  config.cashierId);
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::devEnv,  DeviceUtils::boolToDeviceBoolStr(config.devEnv));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::phoneConFeedback,  DeviceUtils::boolToDeviceBoolStr(config.phoneConFeedback));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::logLevel,  Utils::numberToHexString(config.logLevel));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::connectionRole, DeviceUtils::connectionRoleToString(config.connectionRole));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::mode, DeviceUtils::operationModeToString(config.mode));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::mobileAppsUUIDs, DeviceUtils::mobileAppsUUIDsToString(config.mobileAppsUUIDs));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::txPower,  Utils::numberToHexString((uint8_t) config.txPower));
    })
    .thenPromise([&]() {
      return serial.set(DeviceParameter::tapSensitivity,  Utils::numberToHexString((uint8_t) config.tapSensitivity));
    })
    .thenPromise([&]() {
      return serial.commit(config.signature);
    })
    .then([&]() { 
      writePromise.set_value();
    })
    .catchError([&writePromise](const exception_ptr& ex) { writePromise.set_exception(ex); });

  writeDone.wait();

  writeDone.get(); //raise exception if is set_exception
}

static void updateDeviceConfig(DeviceInfo& deviceInfo, PortalHostEnv env) {
  if (!isValidDeviceInfo(deviceInfo)) {
      throw runtime_error("Unable to get device info. Try to reconnect the device.");
  }
  PortalBackend backend(env);
  auto config = getDevicePortalConfig(backend, deviceInfo);
  Logger configLogger("config");
  JetBeep::SerialDevice serial;
  serial.open(deviceInfo.systemPath);
  configLogger.i() << "Applying new configuration ..." << Logger::endl;
  writeDeviceConfig(config, serial);
  serial.close();
  DFU::SyncSerialDevice syncSerialDevice = DFU::SyncSerialDevice();
  configLogger.i() << "Reseting the device ..." << Logger::endl;
  delay_flash_write(); //wait until flash write is completed
  syncSerialDevice.open(deviceInfo.systemPath);
  syncSerialDevice.reset();
  syncSerialDevice.close();
  updateDevicePortalConfig(backend, deviceInfo);
}

int main(int argc, char* argv[]) {
  Logger::coutEnabled = true;
  PortalHostEnv env = PortalHostEnv::Production;
  bool updateFwDone = false;
  bool updateConfigDone = false;
  bool doFwUpdate = true;
  bool doConfiguration = true;
  int err_code = 0;
  Logger::level = LoggerLevel::info;
  string utilityVersion = JETBEEP_VERSION;

  for (int i = 1; i < argc; i++) {
    string param = string(argv[i]);
    if (param == "--help" || param == "-h") {
      cout << "JetBeep device firmware and configuration update utility.\n";
      cout << "Version: " << utilityVersion << "\n\n";
      cout << "Optional parameters: " << "\n";
      cout << "--dev         - dev server configuration." << "\n";
      cout << "--log=verbose or --log=debug - to provide more details during update." << "\n";
      cout << "--config-only - skip firware update. Same if no fw .zip were found." << "\n";
      cout << "--dfu-only    - skip config update." << "\n";
      cout << "--help        - display help info." << "\n\n";
      cout << "Usage: " << "\n";
      cout << "To update firmware place fw .zip packages in the same folder as utility binary." << "\n";
      cout << "Device config will be updated after firmware and require internet connection." << "\n";
      return 0;
    } else if (param == "--log=verbose") {
      Logger::level = LoggerLevel::verbose;
    } else if (param == "--log=debug") {
      Logger::level = LoggerLevel::debug;
    } else if (param == "--version") {
      cout << utilityVersion << "\n";
      return 0;
    } else if (param == "--config-only") {
      doConfiguration = true;
      doFwUpdate = false;
    } else if (param == "--dfu-only") {
      doFwUpdate = true;
      doConfiguration = false;
    } else if (param == "--dev") {
      env = PortalHostEnv::Development;
    } else {
      cout << "Invalid parameter supplied: [" << param << "]\n";
      return -1;
    }
  }

  DeviceInfo deviceInfo;
  vector<PackageInfo> zipPackages;

  auto onError = [&](const exception& e) -> int {
    l.e() << e.what() << Logger::endl;
    l.i() << "Press Enter to exit " << Logger::endl;
    (void)cin.get();
    return -1;
  };
  try {
    deviceInfo = getDeviceInfo();
    if (deviceInfo.deviceId == 0 && deviceInfo.bootState == DeviceBootState::APP) {
      l.e() << "Unable to proceed. The connected device is not initially configured (blank). Please contact your supplier." << Logger::endl;
      return -1;
    }
  } catch (const exception& e) {
    return onError(e);
  }

  if (doFwUpdate) {
    try {
      zipPackages = findZipPackages();
      for (auto p : zipPackages) {
        l.i() << "Firmware package found: " << p.path << Logger::endl;
      }
      if (zipPackages.size() == 0) {
        l.w() << "No firmware update packages were found!" << Logger::endl;
      } else {
        string prevFwVersion = deviceInfo.version;
        updateFirmwareProcedure(deviceInfo, zipPackages);
        delay_boot();
        deviceInfo = getDeviceInfo(true);
        updateFwDone = prevFwVersion != deviceInfo.version;
        if (!updateFwDone) {
          l.i() << "The firmware version was not changed" << Logger::endl;
        }
      }
    } catch (const DFU::ExtendedError& e) {
      return onError(e);
    } catch (const exception& e) {
      return onError(e);
    }
  }

  if (doConfiguration) {
    try {
      l.i() << "Processing device configuration" << Logger::endl;
      if (!isValidDeviceInfo(deviceInfo)) {
        deviceInfo = getDeviceInfo(true);
      }
      updateDeviceConfig(deviceInfo, env);
      updateConfigDone = true;
    } catch (const exception& e) {
      return onError(e);
    }
  }

  l.i() << "-----------------------------------------------" << Logger::endl;
  l.i() << "Status: Firmware updated " << (updateFwDone ? "YES" : "NO") << ", Config updated "
        << (updateConfigDone ? "YES" : "NO") << Logger::endl;

  l.i() << "Press Enter to exit " << Logger::endl;
  (void)cin.get();

  return 0;
}
