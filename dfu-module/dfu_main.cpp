#include <stdio.h>
#include <string.h>
#include "uart_drv.h"
#include "uart_slip.h"
#include "dfu.h"
#include "logging.h"
#include <future>
#include "../lib/libjetbeep.hpp"
#include "./serial_device.hpp"

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

int main(int argc, char* argv[]) {
  Logger::coutEnabled = true;
  Logger::level = LoggerLevel::verbose;
  int err_code = 0;
  string devicePath;
  DFU::SerialDevice serialDevice = DFU::SerialDevice();

  try {
    devicePath = findJetBeepDeviceCandidate().path;
    serialDevice.open(devicePath);
    l.d() << "Port opened" << Logger::endl;
    auto deviceId = serialDevice.getDeviceId();
    auto fwVer = serialDevice.getFirmwareVer();
    l.d() << "DeviceID: " <<  deviceId << " Firmware: " << fwVer << Logger::endl;
  } catch (const exception& e) {
    l.e() << e.what() << Logger::endl;
    return -1;
  }

  uart_drv_t uart_drv;

  // TODO implement zip and search
  char* portName = NULL;
  char* zipName = NULL;

  logger_set_info_level(LOGGER_INFO_LVL_0);

  uart_drv.p_PortName = portName;

  if (!err_code) {
    err_code = uart_slip_open(&uart_drv);
  }

  if (!err_code) {
    dfu_param_t dfu_param;

    dfu_param.p_uart = &uart_drv;
    dfu_param.p_pkg_file = zipName;
    err_code = dfu_send_package(&dfu_param);
  }

  if (!err_code) {
    int err_code = uart_slip_close(&uart_drv);
  }

  l.i() << "exit" << Logger::endl;
  return err_code;
}
