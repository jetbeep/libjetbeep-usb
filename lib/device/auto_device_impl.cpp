#include "auto_device_impl.hpp"
#include "device_errors.hpp"

#include <functional>

using namespace std;
using namespace JetBeep;

AutoDevice::Impl::Impl(AutoDeviceStateChangeCallback *callback)
: m_callback(callback), m_state(AutoDeviceState::invalid), m_log("autodevice") {
  m_detection.callback = std::bind(&AutoDevice::Impl::onDeviceEvent, this, placeholders::_1, placeholders::_2);
}

AutoDevice::Impl::~Impl() {}

void AutoDevice::Impl::onDeviceEvent(const DeviceDetectionEvent& event, const DeviceCandidate& candidate) {
  auto callback = *m_callback;

  switch (event) {
    case DeviceDetectionEvent::added:
        if (m_state != AutoDeviceState::invalid) {
          m_log.w() << "detected additional device in the system, ignoring.." << Logger::endl;
          return;
        }

        try {
          m_device.open(candidate.path);
          m_candidate = candidate;
          resetState();
        } catch (...) {
          m_log.e() << "unable to open device!" << Logger::endl;
        }
      break;
    case DeviceDetectionEvent::removed:
      if (m_candidate != candidate) {
        return;
      } 

      try {
        m_device.close();          
      } catch (...) {
        m_log.e() << "unable to close device!" << Logger::endl;
      }

      m_state = AutoDeviceState::invalid;
      notifyStateChange(m_state, make_exception_ptr(Errors::DeviceLost()));    
      break;
  }
}

void AutoDevice::Impl::notifyStateChange(AutoDeviceState state, exception_ptr exception) {
  auto callback = *m_callback;

  if (callback) {
    callback(m_state, exception);
  }
}

void AutoDevice::Impl::resetState() {
  m_device.resetState()
    .then([&] {
    m_state = AutoDeviceState::sessionClosed;
    notifyStateChange(m_state, nullptr);
  }).catchError([&] (exception_ptr exception) {
    if (m_state != AutoDeviceState::invalid) {
      m_state = AutoDeviceState::invalid;
      notifyStateChange(m_state, exception);
      // TODO: add 2 seconds timeout and repeat attempts
    }
  });
}