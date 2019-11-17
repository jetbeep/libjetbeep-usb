#include "auto_device.hpp"
#include "auto_device_impl.hpp"

using namespace std;
using namespace JetBeep;

AutoDevice::AutoDevice(): m_impl(new AutoDevice::Impl(&stateCallback, &paymentErrorCallback)) {

}
AutoDevice::~AutoDevice() {}