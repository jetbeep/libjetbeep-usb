#ifndef JETBEEP_MFC_PROVIDER__H
#define JETBEEP_MFC_PROVIDER__H

#include <memory>
#include "../../serial_device.hpp"

namespace JetBeep::NFC {
  namespace MifareClassic {

    enum class MifareIOErrorReason {
      UNKNOWN,
      CARD_REMOVED,
      UNSUPPORTED_CARD_TYPE,
      AUTH_ERROR,
      INTERRUPTED,
      KEY_PARAM_INVALID,
      PARAMS_INVALID,
      DATA_SIZE
    };

    enum class MifareClassicKeyType {
      NONE = 0,
      KEY_A = 1,
      KEY_B = 2
    };

    typedef struct MifareClassicKey {
      char key_data[6];
      MifareClassicKeyType type;
    } NFCMifareClassicKey;

    class MifareClassicProvider {
    public:
      virtual ~MifareClassicProvider();
    private:
      MifareClassicProvider(std::weak_ptr<SerialDevice>);
      class Impl;
      std::unique_ptr<Impl> m_impl;

      friend class AutoDevice;
    };

  } //namespace MifareClassic
} // namespace JetBeep::NFC

#endif //JETBEEP_MFC_PROVIDER__H