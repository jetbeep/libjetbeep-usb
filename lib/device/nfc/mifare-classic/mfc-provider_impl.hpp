#ifndef JETBEEP_MFC_PROVIDER_IMPL__H
#define JETBEEP_MFC_PROVIDER_IMPL__H

#include <boost/beast/core/detail/base64.hpp>
#include "mfc-provider.hpp"
#include "../../serial_device.hpp"

namespace JetBeep::NFC::MifareClassic {
  class MifareClassicProvider::Impl {
  public:
    Impl(std::weak_ptr<SerialDevice>);
    virtual ~Impl();

  private:
    std::weak_ptr<SerialDevice> m_serialDevice_p;
  };
} // namespace JetBeep::NFC::MifareClassic

#endif //JETBEEP_MFC_PROVIDER_IMPL__H