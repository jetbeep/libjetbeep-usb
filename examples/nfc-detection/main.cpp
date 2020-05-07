#include "../../lib/libjetbeep.hpp"
#include <boost/beast/core/detail/base64.hpp>

#include <random>
#include <string>
#include <sstream>

using namespace JetBeep;
using namespace std;

/* Mifare Classic test vars */
#define MFC_TEST_BLOCKNO 61
#define MFC_TEST_KEY_BASE64 "Zku67Rb6"
#define MFC_TEST_KEY_TYPE 1

Logger l("ex-main");

JetBeep::AutoDevice autoDevice;

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

void performMifareClassicRW() {
  if (!autoDevice.isNFCDetected()) {
    return;
  }
  /* read test */
  l.i() << "Reading Mifare block " << MFC_TEST_BLOCKNO << " ..."  << Logger::endl;

  l.i() << "Content of block "<<MFC_TEST_BLOCKNO<<" (base64): " << "" << Logger::endl;

  string input;
  l.i() << "Write 'yes' to perform writing test"  << Logger::endl;
  getline(cin, input);
  if (input != "yes") {
    return;
  }

  /* write test */
  l.i() << "Please enter base64 content (24 chars) for block "<<MFC_TEST_BLOCKNO<<""  << Logger::endl;
  bool contentValid = false;
  do {
    getline(cin, input);
    if (input.size() == 24) {
      char buf[18];
      auto result = boost::beast::detail::base64::decode(buf, input.c_str(), 24);
      contentValid = result.first == 16;
    }
    if (!contentValid) {
      l.i() << "Invalid input, please try again"  << Logger::endl;
    }
  } while (!contentValid);

  l.i() << "Writing data to block "<<MFC_TEST_BLOCKNO<<" ..."  << Logger::endl;
}

void handleDetectedCard(const NFC::DetectionEventData& data) {
  string typeStr;
  std::stringstream metaStream;
  bool isMifareClassic = false;
  switch (data.cardType) {
  case JetBeep::NFC::CardType::EMV_CARD: {
      typeStr = "Back card";
      auto parts = Utils::splitString(data.meta, "d");
      string PAN = parts[0];
      string expDateYear = parts[1].substr(0, 2);
      string expDateMonth = parts[1].substr(2, 2);
      metaStream << ", PAN: " << PAN << ", expiration date: year " << expDateYear << " month " << expDateMonth;
    };
      break;
    case JetBeep::NFC::CardType::MIFARE_CLASSIC_1K: {
      typeStr = "Mifare classic 1K";
      metaStream << " memory 1024, blocks 64, UUID/NUID: " << data.meta;
      isMifareClassic = true;
    };
      break;
    case JetBeep::NFC::CardType::MIFARE_CLASSIC_4K: {
      typeStr = "Mifare classic 4K";
      metaStream << " memory 4096, blocks 256, UUID/NUID: " << data.meta;
      isMifareClassic = true;
    };
      break;
    case JetBeep::NFC::CardType::MIFARE_PLUS_2K:
    case JetBeep::NFC::CardType::MIFARE_PLUS_4K: {
      typeStr = "Mifare Plus";
      metaStream << "UUID/NUID: " << data.meta;
      break;
    }
    case JetBeep::NFC::CardType::MIFARE_DESFIRE_2K:
    case JetBeep::NFC::CardType::MIFARE_DESFIRE_4K:
    case JetBeep::NFC::CardType::MIFARE_DESFIRE_8K: {
      typeStr = "Mifare Desfire";
      metaStream << "UUID/NUID: " << data.meta;
      break;
    }
    default:
      typeStr = "Unsupported";
  }
  l.i() << "NFC card detected, type: " << typeStr << metaStream.str() << Logger::endl;

  if (isMifareClassic) {
    performMifareClassicRW();
  }
}

void onNFCEvent(const SerialNFCEvent& event, const NFC::DetectionEventData& data) {
  if (event == SerialNFCEvent::detected) {
    handleDetectedCard(data);
  } else {
    l.e() << "NFC card removed" << Logger::endl;
  }
}

int main() {
  string input;
  Logger::coutEnabled = true;
  Logger::level = LoggerLevel::verbose;

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

  cout << "NFC detection started\n";

  while (true) {
    getline(cin, input);
    if (input == "exit") {
      break;
    }
  }
  return 0;
}