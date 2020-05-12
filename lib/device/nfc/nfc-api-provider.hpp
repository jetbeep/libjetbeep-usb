#ifndef JETBEEP_NFC_API_PROVIDER__H
#define JETBEEP_NFC_API_PROVIDER__H

#include <memory>
#include "../serial_device.hpp"

namespace JetBeep::NFC {
  class NFCApiProvider {
  public:
    virtual ~NFCApiProvider();

  protected:
    NFCApiProvider(std::shared_ptr<SerialDevice> &);
    std::weak_ptr<SerialDevice> m_serial_p;
  };
} // namespace JetBeep::NFC

#endif //JETBEEP_NFC_API_PROVIDER__H