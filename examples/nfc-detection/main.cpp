#include "../../lib/libjetbeep.hpp"

#include <random>
#include <string>

using namespace JetBeep;
using namespace std;

Logger l("ex-main");

void onStateChange(AutoDeviceState state, std::exception_ptr error) {
  switch (state) {
  case AutoDeviceState::firmwareVersionNotSupported:
    l.i() << "changed state to: firmwareVersionNotSupported" << Logger::endl;
    break;
  case AutoDeviceState::invalid:
    l.i() << "changed state to: invalid" << Logger::endl;
    break;
  case AutoDeviceState::sessionClosed:
    l.i() << "changed state to: sessionClosed" << Logger::endl;
    break;
  case AutoDeviceState::sessionOpened:
    l.i() << "changed state to: sessionOpened" << Logger::endl;
    break;
  }
}

void onMobileEvent(const JetBeep::SerialMobileEvent& event) {
  if (event == SerialMobileEvent::connected) {
    l.i() << "connected" << Logger::endl;
  } else {
    l.e() << "disconnected" << Logger::endl;
  }
}

void onNFCEvent(const SerialNFCEvent& event, const NFCDetectionEventData& data) {
  if (event == SerialNFCEvent::detected) {
    l.i() << "NFC card detected, type: " << data.cardType << ", meta: " << data.meta << Logger::endl;
  } else {
    l.e() << "NFC card removed" << Logger::endl;
  }
}

int main() {
  string input;
  Logger::coutEnabled = true;
  Logger::level = LoggerLevel::verbose;

  JetBeep::AutoDevice autoDevice;
  autoDevice.mobileCallback = onMobileEvent;
  autoDevice.stateCallback = onStateChange;
  autoDevice.nfcEventCallback = onNFCEvent;
  autoDevice.start();

  cout << "USB Device connected?\n";
  getline(cin, input);

  auto state = autoDevice.state();
  if (state == AutoDeviceState::sessionOpened) {
    autoDevice.closeSession();
  }
  autoDevice.enableNFC();
  autoDevice.disableBluetooth();

  autoDevice.openSession();
  
  while (true) {
    getline(cin, input);
    if (input == "exit") {
      break;
    }
  }
  return 0;
}