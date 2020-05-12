#ifndef JETBEEP_MFC_PROVIDER__H
#define JETBEEP_MFC_PROVIDER__H

#include <memory>
#include "../../serial_device.hpp"
#include "../../auto_device.hpp"
#include "../nfc-api-provider.hpp"

#define MFC_BLOCK_SIZE 16
#define MFC_KEY_SIZE 6

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

    class MifareIOException : public ::JetBeep::Errors::InvalidResponseWithReason {
    public:
      MifareIOException(std::string code): InvalidResponseWithReason(code) {};
      MifareIOErrorReason getIOErrorReason() {
        if (m_code == "unknown" || m_code == "internal") {
          return MifareIOErrorReason::UNKNOWN;
        }
        if (m_code == "card_removed") {
          return MifareIOErrorReason::CARD_REMOVED;
        }
        if (m_code == "unsupported_type") {
          return MifareIOErrorReason::UNSUPPORTED_CARD_TYPE;
        }
        if (m_code == "auth") {
          return MifareIOErrorReason::AUTH_ERROR;
        }
        if (m_code == "io") {
          return MifareIOErrorReason::INTERRUPTED;
        }
        if (m_code == "key_param") {
          return MifareIOErrorReason::KEY_PARAM_INVALID;
        }
        if (m_code == "params") {
          return MifareIOErrorReason::PARAMS_INVALID;
        }
        if (m_code == "data_size") {
          return MifareIOErrorReason::DATA_SIZE;
        }
        return MifareIOErrorReason::UNKNOWN;
      }
    };


    enum class MifareClassicKeyType {
      NONE = 0,
      KEY_A = 1,
      KEY_B = 2
    };

    typedef struct MifareClassicKey {
      char key_data[MFC_KEY_SIZE];
      MifareClassicKeyType type;
    } NFCMifareClassicKey;

    typedef struct MifareBlockContent {
      char data[MFC_BLOCK_SIZE];
      int blockNo;
    } MifareBlockContent;

    class MifareClassicProvider: public NFCApiProvider {
    public:
      virtual ~MifareClassicProvider();
      JetBeep::Promise<void> readBlock(int blockNo, MifareBlockContent & content, const MifareClassicKey *key = nullptr);
      JetBeep::Promise<void> writeBlock(const MifareBlockContent & content, const MifareClassicKey *key = nullptr);
      MifareClassicProvider(std::shared_ptr<SerialDevice> &, DetectionEventData &cardInfo);
    private:
      class Impl;
      std::unique_ptr<Impl> m_impl;
    };

  } //namespace MifareClassic
} // namespace JetBeep::NFC

#endif //JETBEEP_MFC_PROVIDER__H