#include "../utils/platform.hpp"
#include "mfc-provider.hpp"
#include "mfc-provider_impl.hpp"

using namespace std;
using namespace JetBeep::NFC::MifareClassic;

MifareClassicProvider::MifareClassicProvider(std::shared_ptr<SerialDevice>& device_p, DetectionEventData& cardInfo)
  : NFCApiProvider(device_p, cardInfo),
    m_impl(new Impl(cardInfo)){};

MifareClassicProvider::~MifareClassicProvider() {}


JetBeep::Promise<void> MifareClassicProvider::readBlock(int blockNo, MifareBlockContent & content, const MifareClassicKey *key){
  auto serial = m_serial_p.lock();
  if (!serial) {
    throw Errors::NullPointerError();
  }
  return m_impl->readBlock(serial, blockNo, key, content);
}

JetBeep::Promise<void> MifareClassicProvider::writeBlock(const MifareBlockContent & content, const MifareClassicKey *key) const {
  auto serial = m_serial_p.lock();
  if (!serial) {
    throw Errors::NullPointerError();
  }
  return m_impl->writeBlock(serial, content, key);
}
