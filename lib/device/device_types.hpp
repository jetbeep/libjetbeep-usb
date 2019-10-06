#ifndef JETBEEP_DEVICE_TYPES_H
#define JETBEEP_DEVICE_TYPES_H

#include <string>
#include <vector>

namespace JetBeep {
  enum class DeviceEvent {
    deviceError,
    protocolError,
    mobileConnected,
    mobileDisconnected,
    barcodes,
    paymentSuccessful,
    paymentError,
    paymentToken
  };

  struct Barcode {
    std::string value;
    int type;
  };

  typedef void (*DeviceCallback)(const DeviceEvent &);
}

#endif