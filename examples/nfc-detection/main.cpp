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
#define MFC_TEST_KEY_TYPE NFC::MifareClassic::MifareClassicKeyType::KEY_A

#define MFC_TEST_WRITE_ENABLED true
#define MFC_TEST_WRITE_CONTENT "ABEiM0RVZneImaq7zN3u/w=="

Logger l("ex-main");

JetBeep::AutoDevice * autoDevice_p;

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
NFC::MifareClassic::MifareBlockContent rBlockContent;
NFC::MifareClassic::MifareBlockContent wBlockContent;
NFC::MifareClassic::MifareClassicKey key;

void performMifareClassicRW() {
  if (!autoDevice_p->isNFCDetected()) {
    return;
  }

  auto nfcApiProvider = autoDevice_p->createNFCApiProvider();
  auto mifareApi = std::dynamic_pointer_cast<NFC::MifareClassic::MifareClassicProvider>(nfcApiProvider);

  std::string keyBase64 = MFC_TEST_KEY_BASE64;
  auto mifareKeySizeBase64 = keyBase64.size();

  // prepare key
  key.type = MFC_TEST_KEY_TYPE;
  boost::beast::detail::base64::decode(key.key_data, keyBase64.c_str(), mifareKeySizeBase64);

  /* read test */
  l.i() << "Reading Mifare block " << MFC_TEST_BLOCKNO << " ..." << Logger::endl;

  auto onIOError = [&](const std::exception_ptr& error) {
      try {
        rethrow_exception(error);
      } catch (JetBeep::NFC::MifareClassic::MifareIOException& error) {
        l.e() << "Read/Write failed. " << error.what() << Logger::endl;
        switch (error.getIOErrorReason()) {
        case JetBeep::NFC::MifareClassic::MifareIOErrorReason::AUTH_ERROR:
          l.e() << "Invalid Mifare sector key" << Logger::endl;
          break;
        case JetBeep::NFC::MifareClassic::MifareIOErrorReason::CARD_REMOVED:
          l.e() << "Card removed before operation completed" << Logger::endl;
          break;
        case JetBeep::NFC::MifareClassic::MifareIOErrorReason::UNSUPPORTED_CARD_TYPE:
          l.e() << "Card type is not Mifare Classic" << Logger::endl;
          break;
        case JetBeep::NFC::MifareClassic::MifareIOErrorReason::DATA_SIZE:
          l.e() << "Invalid content data size" << Logger::endl;
          break;
        case JetBeep::NFC::MifareClassic::MifareIOErrorReason::INTERRUPTED:
          l.e() << "IO command interrupted" << Logger::endl;
          break;
        case JetBeep::NFC::MifareClassic::MifareIOErrorReason::KEY_PARAM_INVALID:
          l.e() << "Invalid key format" << Logger::endl;
          break;
        case JetBeep::NFC::MifareClassic::MifareIOErrorReason::UNKNOWN:
          break;
        case JetBeep::NFC::MifareClassic::MifareIOErrorReason::PARAMS_INVALID:
          l.e() << "Block # is out of bounds" << Logger::endl;
          break;
        }
      } catch (const std::exception error) {
        l.e() << "Read/Write error:" << error.what() << Logger::endl;
      }catch (...) {
        l.e() << "Read/Write failed due to unknown error" << Logger::endl;
      }
    };

  mifareApi->readBlock(MFC_TEST_BLOCKNO, rBlockContent, &key /* pass nullptr to use default (factory) Mifare key */)
    .thenPromise([&, mifareApi]() -> Promise<void> {
      std::string base64Result;
      base64Result.resize(boost::beast::detail::base64::encoded_size(MFC_BLOCK_SIZE));
      boost::beast::detail::base64::encode((void*)base64Result.c_str(), rBlockContent.data, MFC_BLOCK_SIZE);
      l.i() << "Content of block " << MFC_TEST_BLOCKNO << " (base64): " << base64Result << Logger::endl;

      /* write test */
      if (MFC_TEST_WRITE_ENABLED) {
        auto mifareBlockSizeBase64 = boost::beast::detail::base64::encoded_size(MFC_BLOCK_SIZE);
        auto content = string(MFC_TEST_WRITE_CONTENT);
        if (content.size() == mifareBlockSizeBase64) {
          char buf[18 /*boost::beast::detail::base64::decoded_size(mifareBlockSizeBase64)*/];
          auto result = boost::beast::detail::base64::decode(buf, content.c_str(), mifareBlockSizeBase64);
          auto contentValid = result.first == MFC_BLOCK_SIZE;
          if (!contentValid) {
            l.e() << "Invalid content" << Logger::endl;
            throw runtime_error("invalid content");
          }
          //setting up content
          memcpy(wBlockContent.data, buf, MFC_BLOCK_SIZE);
          wBlockContent.blockNo = MFC_TEST_BLOCKNO;
        }

        l.i() << "Writing data to block " << MFC_TEST_BLOCKNO << " ..." << Logger::endl;
        return mifareApi->writeBlock(wBlockContent, &key /* pass nullptr to use default (factory) Mifare key */);
      } else {
        auto p = Promise<void>();
        p.resolve();
        return p;
      }
    })
    .then([&]() { l.i() << "Write success " << Logger::endl; })
    .catchError(onIOError);
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

  JetBeep::AutoDevice autoDevice;
  autoDevice_p = &autoDevice;

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