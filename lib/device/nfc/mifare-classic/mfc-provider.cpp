#include "../utils/platform.hpp"
#include "mfc-provider.hpp"
#include "mfc-provider_impl.hpp"

using namespace std;
using namespace JetBeep::NFC::MifareClassic;

MifareClassicProvider::MifareClassicProvider(std::shared_ptr<SerialDevice>& device_p, DetectionEventData& cardInfo)
  : NFCApiProvider(device_p),
    m_impl(new Impl(cardInfo)){};

MifareClassicProvider::~MifareClassicProvider() {}


void MifareClassicProvider::readBlock(const int blockNo, const MifareClassicKey &key, MifareBlockContent & content){
  auto serial = m_serial_p.lock();
  if (!serial) {
    throw Errors::NullPointerError();
  }
  return m_impl->readBlock(serial, blockNo, key, content);
}

void MifareClassicProvider::writeBlock(const MifareBlockContent & content, const MifareClassicKey &key){
  auto serial = m_serial_p.lock();
  if (!serial) {
    throw Errors::NullPointerError();
  }
  return m_impl->writeBlock(serial, content, key);
}
