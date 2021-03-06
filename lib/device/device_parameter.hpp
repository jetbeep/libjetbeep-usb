#ifndef JETBEEP_DEVICE_PARAMETER__H
#define JETBEEP_DEVICE_PARAMETER__H

#include <string>

namespace JetBeep {
  enum class DeviceParameter {
    version,
    shopId,
    deviceId,
    mode,
    pubKey,
    chipId,
    revision,
    paymentPubKey,
    shopKey,
    cashierId,
    txPower,
    tapSensitivity,
    phoneConFeedback,
    proximitySensitivity,
    mobileAppsUUIDs,
    mac,
    devEnv,
    connectionRole,
    logLevel,
    merchantId,
    domainShopId,
    //after 1.4.0
    virtKeyboard,
    //after 1.5.0
    nfc,
    bluetooth
  };
}

#endif