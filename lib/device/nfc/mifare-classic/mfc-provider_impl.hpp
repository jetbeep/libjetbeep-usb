#ifndef JETBEEP_MFC_PROVIDER_IMPL__H
#define JETBEEP_MFC_PROVIDER_IMPL__H

#include <boost/beast/core/detail/base64.hpp>
#include "mfc-provider.hpp"
#include "../../serial_device.hpp"

namespace JetBeep::NFC::MifareClassic {
  class MifareClassicProvider::Impl {
  public:
    Impl(DetectionEventData &);
    virtual ~Impl();

    void readBlock(std::shared_ptr<SerialDevice>, const int, const MifareClassicKey &, MifareBlockContent &);
    void writeBlock(std::shared_ptr<SerialDevice>, const MifareBlockContent & content, const MifareClassicKey &key);
  private:
    DetectionEventData &m_cardInfo;
  };
} // namespace JetBeep::NFC::MifareClassic

#endif //JETBEEP_MFC_PROVIDER_IMPL__H