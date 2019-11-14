#ifndef JETBEEP_AUTO_DEVICE_IMPL__H
#define JETBEEP_AUTO_DEVICE_IMPL__H

#include "auto_device.hpp"
#include "serial_device.hpp"
#include "../detection/detection.hpp"

namespace JetBeep {
  class AutoDevice::Impl {
  public:
    Impl(AutoDeviceStateChangeCallback *callback);
    virtual ~Impl();
  private:
    AutoDeviceStateChangeCallback *m_callback;
    DeviceDetection m_detection;
    SerialDevice m_device;
    
    void onDeviceEvent(const DeviceDetectionEvent& event, const DeviceCandidate& candidate);
  };
}

#endif