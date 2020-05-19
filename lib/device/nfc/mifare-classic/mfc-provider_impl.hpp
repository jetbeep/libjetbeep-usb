#ifndef JETBEEP_MFC_PROVIDER_IMPL__H
#define JETBEEP_MFC_PROVIDER_IMPL__H

#include <boost/beast/core/detail/base64.hpp>
#include "mfc-provider.hpp"
#include "../../serial_device.hpp"
#include <mutex>

namespace JetBeep::NFC::MifareClassic {
  class MifareClassicProvider::Impl {
  public:
    Impl(DetectionEventData &);
    virtual ~Impl();

    Promise<void> readBlock(std::shared_ptr<SerialDevice>, int, const MifareClassicKey *, MifareBlockContent &);
    Promise<void> writeBlock(std::shared_ptr<SerialDevice>, const MifareBlockContent & content, const MifareClassicKey *);
  private:
    DetectionEventData &m_cardInfo;
    std::recursive_mutex m_mutex;
    JetBeep::Promise<void> m_result_promise;
  };
} // namespace JetBeep::NFC::MifareClassic

#endif //JETBEEP_MFC_PROVIDER_IMPL__H