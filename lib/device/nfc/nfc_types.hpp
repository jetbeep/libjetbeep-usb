#ifndef JETBEEP_NFC_TYPES_H
#define JETBEEP_NFC_TYPES_H

#include <string>

namespace JetBeep {
  enum class SerialNFCEvent { detected, removed };

  namespace NFC {
    enum class CardType {
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

    typedef struct DetectionEventData {
      CardType cardType;
      std::string meta;
    } DetectionEventData;

    enum class DetectionErrorReason {
      UNKNOWN,
      MULTIPLE_CARDS,
      UNSUPPORTED
    };
  } //namespace NFC
} // namespace JetBeep

#endif // JETBEEP_NFC_TYPES_H