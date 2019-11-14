#include "auto_device_impl.hpp"

using namespace std;
using namespace JetBeep;

AutoDevice::Impl::Impl(AutoDeviceStateChangeCallback *callback): m_callback(callback) {

}

AutoDevice::Impl::~Impl() {}

void AutoDevice::Impl::onDeviceEvent(const DeviceDetectionEvent& event, const DeviceCandidate& candidate) {
  
}