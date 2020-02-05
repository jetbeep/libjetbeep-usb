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

static DeviceInfo getDeviceInfo() {
  DeviceInfo deviceInfo;
  deviceInfo.systemPath = findJetBeepDeviceCandidate().path;

  // TODO
  return deviceInfo;
}

static void updateFirmwareProcedure(DeviceInfo& deviceInfo, vector<string>& zipPackages) {
  int err_code = 0;
  Logger dfuLogger("dfu");
  logger_set_backend(&dfuLogger);

  DFU::SyncSerialDevice syncSerialDevice = DFU::SyncSerialDevice();
  uart_drv_t uart_drv = {&syncSerialDevice};

  syncSerialDevice.open(deviceInfo.systemPath);

  if (deviceInfo.bootState == DeviceBootState::APP) {
    syncSerialDevice.enterDFUMode();
    delay_boot();
  }

  l.i() << "Starting firmware update procedure." << Logger::endl;
  for (string zipPath : zipPackages) {
    // test that device in bootloader mode
    err_code = dfu_serial_ping(&uart_drv, 0x04);
    if (err_code == 0) {
      deviceInfo.bootState = DeviceBootState::BOOTLOADER;
    } else {
      break;
    }
    
    l.d() << "Processing firmware package: " << zipPath << Logger::endl;

    dfu_param_t dfu_param;
    dfu_param.p_uart = &uart_drv;
    dfu_param.p_pkg_file = (char*)zipPath.c_str();
    err_code = dfu_send_package(&dfu_param);
  }
  if (err_code != 0) {
    throw runtime_error("Unable to complete firmware update procedure.");
  }
}

int main(int argc, char* argv[]) {
  Logger::coutEnabled = true;
  Logger::level = LoggerLevel::verbose;
  bool updateFwDone = false;
  bool updateConfigDone = false;
  int err_code = 0;
  DeviceInfo deviceInfo;
  vector<string> zipPackages;

  auto onError = [&](const exception& e) -> int {
    l.e() << e.what() << Logger::endl;
    return -1;
  };
  try {
    deviceInfo = getDeviceInfo();
    zipPackages = findZipPackages();
    for (auto p : zipPackages) {
      l.i() << "Firmware package found: " << p << Logger::endl;
    }
  } catch (const exception& e) {
    return onError(e);
  }

  if (zipPackages.size() == 0) {
    l.w() << "No firmware update packages were found!" << Logger::endl;
  } else {
    try {
      updateFirmwareProcedure(deviceInfo, zipPackages);
      updateFwDone = true;
    } catch (const exception& e) {
      l.e() << getExtendedErrorMsg() << Logger::endl;
      return onError(e);
    }
  }

  {
    l.i() << "Processing device configuration" << Logger::endl;
    // TODO
  }

  l.i() << "-----------------------------------------------" << Logger::endl;
  l.i() << "Status: Firmware updated " << (updateFwDone ? "YES" : "NO") << ", Config updated "
        << (updateConfigDone ? "YES" : "NO") << Logger::endl;
  l.i() << "Press Enter to exit " << Logger::endl;
  (void)cin.get();

  return 0;
}
