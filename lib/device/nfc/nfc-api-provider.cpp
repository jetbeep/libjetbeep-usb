#include "../utils/platform.hpp"
#include "nfc-api-provider.hpp"

using namespace std;
using namespace JetBeep::NFC;

NFCApiProvider::NFCApiProvider(std::shared_ptr<SerialDevice> &device_p, DetectionEventData& cardInfo): m_cardInfo_p(cardInfo) {
  m_serial_p = std::weak_ptr<SerialDevice>(device_p);
}

NFCApiProvider::~NFCApiProvider() {}
