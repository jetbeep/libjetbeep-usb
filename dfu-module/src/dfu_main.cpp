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

static DeviceInfo getDeviceInfo() {
  DeviceInfo deviceInfo;
  deviceInfo.systemPath = findJetBeepDeviceCandidate().path;
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
    .catchError([&infoReadPromise](const exception_ptr& ex) { infoReadPromise.set_exception(ex); });

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

  for (PackageInfo pkg : zipPackages) {
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
      } else if (extErrorCode != (int)NRF_DFU_EXT_ERROR::NO_ERROR) {
        throw DFU::ExtendedError(extErrorCode);
      } else {
        onError();
      }
    }
  }
}

static DeviceConfig getDevicePortalConfig(PortalBackend& backend, DeviceInfo& deviceInfo) {
  std::promise<DeviceConfig> reqPromise;
  auto reqFuture = reqPromise.get_future();
  DeviceConfigRequest request = {.chipId = deviceInfo.chipId};

  backend.getDeviceConfig(request)
    .then([&reqPromise](DeviceConfigResponse res) { reqPromise.set_value(res.config); })
    .catchError([&reqPromise](const exception_ptr& ex) { reqPromise.set_exception(ex); });

  reqFuture.wait();
  return reqFuture.get();
}

static void updateDevicePortalConfig(PortalBackend& backend, DeviceInfo& deviceInfo) {
  std::promise<void> reqPromise;
  auto reqFuture = reqPromise.get_future();
  DeviceConfigUpdateRequest request = {.chipId = deviceInfo.chipId, .fwVersion = deviceInfo.version};

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
}

static void updateDeviceConfig(DeviceInfo& deviceInfo) {
  PortalBackend backend(PortalHostEnv::Production);
  auto config = getDevicePortalConfig(backend, deviceInfo);
  Logger dfuLogger("config");
  JetBeep::SerialDevice serial;
  serial.open(deviceInfo.systemPath);
  writeDeviceConfig(config, serial);
  updateDevicePortalConfig(backend, deviceInfo);
}

int main(int argc, char* argv[]) {
  Logger::coutEnabled = true;
  bool updateFwDone = false;
  bool updateConfigDone = false;
  bool doFwUpdate = false;
  bool doConfiguration = true;
  int err_code = 0;
  Logger::level = LoggerLevel::info;

  for (int i = 1; i < argc; i++) {
    string param = string(argv[i]);
    if (param == "--help") {
      //TODO list params
      return 0;
    } else if (param == "--log=verbose") {
      Logger::level = LoggerLevel::verbose;
    } else if (param == "--log=debug") {
      Logger::level = LoggerLevel::debug;
    } else if (param == "--version") {
      //TODO display version
    } else if (param == "--config-only") {
      doConfiguration = true;
       doFwUpdate = false;
    } else if (param == "--dfu-only") {
      doFwUpdate = true;
      doConfiguration = false;
    } else {
      cout << "Invalid parameter supplied: [" << param << "]\n";
      return -1;
    }
  }

  DeviceInfo deviceInfo;
  vector<PackageInfo> zipPackages;

  auto onError = [&](const exception& e) -> int {
    l.e() << e.what() << Logger::endl;
    return -1;
  };
  try {
    deviceInfo = getDeviceInfo();
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
        deviceInfo = getDeviceInfo();
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
    l.i() << "Processing device configuration" << Logger::endl;
    updateDeviceConfig(deviceInfo);
    updateConfigDone = true;
  }

  l.i() << "-----------------------------------------------" << Logger::endl;
  l.i() << "Status: Firmware updated " << (updateFwDone ? "YES" : "NO") << ", Config updated "
        << (updateConfigDone ? "YES" : "NO") << Logger::endl;

  l.i() << "Press Enter to exit " << Logger::endl;
  (void)cin.get();

  return 0;
}
