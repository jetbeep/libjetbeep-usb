#include <stdio.h>
#include <string.h>
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



int main(int argc, char* argv[]) {
  Logger::coutEnabled = true;
  Logger::level = LoggerLevel::verbose;
  int err_code = 0;
  string devicePath;
  DFU::SerialDevice serialDevice = DFU::SerialDevice();
  vector<string> zipPackages;

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
    l.e() << e.what() << Logger::endl;
    serialDevice.close();
    return -1;
  }

  string zipPath = zipPackages.at(0);

  logger_set_info_level(LOGGER_INFO_LVL_0);

  uart_drv_t uart_drv = {&serialDevice};
  if (!err_code) {
    dfu_param_t dfu_param;

    dfu_param.p_uart = &uart_drv;
    dfu_param.p_pkg_file = (char*)zipPath.c_str();
    //err_code = dfu_send_package(&dfu_param);
  }

  serialDevice.close();

  l.i() << "exit" << Logger::endl;
  return err_code;
}
