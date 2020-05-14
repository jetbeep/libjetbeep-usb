#ifndef JETBEEP_NFC_API_PROVIDER__H
#define JETBEEP_NFC_API_PROVIDER__H

#include <memory>
#include "../serial_device.hpp"

namespace JetBeep::NFC {
  class NFCApiProvider {
  public:
    virtual ~NFCApiProvider();
    DetectionEventData& getNFCCardInfo() {
      return m_cardInfo_p;
    }
  protected:
    NFCApiProvider(std::shared_ptr<SerialDevice> &, DetectionEventData&);
    std::weak_ptr<SerialDevice> m_serial_p;
    DetectionEventData& m_cardInfo_p;
  };
} // namespace JetBeep::NFC

#endif //JETBEEP_NFC_API_PROVIDER__H