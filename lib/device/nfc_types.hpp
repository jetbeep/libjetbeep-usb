#ifndef JETBEEP_NFC_TYPES_H
#define JETBEEP_NFC_TYPES_H

#include <string>

namespace JetBeep {
  enum class SerialNFCEvent { detected, removed };

  enum class NFCCardType {
    UNKNOWN = 0,
    EMV_CARD = 1,
    MIFARE_CLASSIC_1K = 2,
    MIFARE_CLASSIC_4K = 3,
    MIFARE_PLUS_2K = 4,
    MIFARE_PLUS_4K = 5,
    MIFARE_DESFIRE_2K = 6,
    MIFARE_DESFIRE_4K = 7,
    MIFARE_DESFIRE_8K = 8
  };

  typedef struct NFCDetectionEventData {
    NFCCardType cardType;
    std::string meta;
  } NFCDetectionEventData;

  enum class NFCDetectionErrorReason {
    UNKNOWN,
    MULTIPLE_CARDS,
    UNSUPPORTED
  };

  enum class NFCMifareIOErrorReason {
    UNKNOWN,
    CARD_REMOVED,
    UNSUPPORTED_CARD_TYPE,
    AUTH_ERROR,
    INTERRUPTED,
    KEY_PARAM_INVALID,
    PARAMS_INVALID,
    DATA_SIZE
  };

} // namespace JetBeep

#endif // JETBEEP_NFC_TYPES_H