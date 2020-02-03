#include <stdio.h>
#include <string.h>
#include <iostream>
#include "uart_drv.h"
#include "delay_connect.h"
#include "uart_slip.h"
#include "dfu.h"
#include "logging.h"
#include <future>
#include "../lib/libjetbeep.hpp"
#include "./serial_device.hpp"
#include "./packages_search.hpp"

using namespace std;
using namespace JetBeep;

Logger l("main");

std::promise<DeviceCandidate> detectionPromise;

static void handleDetectionEvent(const DeviceDetectionEvent& event, const DeviceCandidate& candidate) {
  if (event == DeviceDetectionEvent::added) {
    detectionPromise.set_value(candidate);
  } else {
    detectionPromise.set_exception(make_exception_ptr(std::runtime_error("device removed")));
  }
}

static DeviceCandidate findJetBeepDeviceCandidate() {
  DeviceDetection deviceDetection;
  deviceDetection.callback = handleDetectionEvent;
  auto detectionReady = detectionPromise.get_future();
  deviceDetection.start();
  detectionReady.wait();
  deviceDetection.stop();
  const DeviceCandidate& candidate = detectionReady.get();
  l.i() << "Found JetBeep device:" << candidate.path << " vid: " << candidate.vid << " pid: " << candidate.pid << Logger::endl;
  return candidate;
}

static void prepareDevice(DFU::SerialDevice& serialDevice) {
  if (serialDevice.isBootloaderMode()) {
    return;
  }
  auto deviceId = serialDevice.getDeviceId();
  auto fwVer = serialDevice.getFirmwareVer();
  l.d() << "DeviceID: " << deviceId << " Firmware: " << fwVer << Logger::endl;
  serialDevice.enterDFUMode();
  delay_boot();
}

static void updateFirmwareProcedure(DFU::SerialDevice& serialDevice, vector<string>& zipPackages) {
  int err_code = 0;
  Logger dfuLogger("dfu");
  logger_set_backend(&dfuLogger);

  for (string zipPath : zipPackages) {

    uart_drv_t uart_drv = {&serialDevice};
    if (!err_code) {
      dfu_param_t dfu_param;

      dfu_param.p_uart = &uart_drv;
      dfu_param.p_pkg_file = (char*)zipPath.c_str();
      err_code = dfu_send_package(&dfu_param);
    }
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
  string devicePath;
  DFU::SerialDevice serialDevice = DFU::SerialDevice();

  vector<string> zipPackages;

  auto onError = [&](const exception& e) -> int{
      l.e() << e.what() << Logger::endl;
      serialDevice.close();
      return -1;
  };
  try {
    zipPackages = findZipPackages();
    for (auto p : zipPackages) {
      l.i() << "Package found: " << p << Logger::endl;
    }
    devicePath = findJetBeepDeviceCandidate().path;
    serialDevice.open(devicePath);
    l.d() << "Port opened" << Logger::endl;
    prepareDevice(serialDevice);
  } catch (const exception& e) {
    return onError(e);
  }

  if (zipPackages.size() == 0) {
    l.w() << "No firmware update packages were found!" << Logger::endl;
  } else {
    try {
      updateFirmwareProcedure(serialDevice, zipPackages);
      updateFwDone = true;
    } catch (const exception& e) {
      return onError(e);
    }
  }

  {
    l.i() << "Processing device configuration" << Logger::endl;
    // TODO
  }

  serialDevice.close();


  l.i() << "-----------------------------------------------" << Logger::endl;
  l.i() << "Status: Firmware updated " <<(updateFwDone ? "YES" : "NO") 
        << ", Config updated " << (updateConfigDone ? "YES" : "NO") << Logger::endl;
  l.i() << "Press Enter to exit "<< Logger::endl;
  (void) cin.get();

  return 0;
}
