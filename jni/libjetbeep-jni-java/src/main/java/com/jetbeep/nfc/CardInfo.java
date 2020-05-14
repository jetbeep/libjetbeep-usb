package com.jetbeep.nfc;

public class CardInfo {
  public enum Type {
    UNKNOWN,
    EMV_CARD,
    MIFARE_CLASSIC_1K,
    MIFARE_CLASSIC_4K,
    MIFARE_PLUS_2K,
    MIFARE_PLUS_4K,
    MIFARE_DESFIRE_2K,
    MIFARE_DESFIRE_4K,
    MIFARE_DESFIRE_8K;

    
    public static Type fromInt(int value) {
      switch (value) {
        case 0:
          return Type.UNKNOWN;
        case 1:
          return Type.EMV_CARD;
        case 2:
          return Type.MIFARE_CLASSIC_1K;
        case 3:
          return Type.MIFARE_CLASSIC_4K;
        case 4:
          return Type.MIFARE_PLUS_2K;
        case 5:
          return Type.MIFARE_PLUS_4K;
        case 6:
          return Type.MIFARE_DESFIRE_2K;
        case 7:
          return Type.MIFARE_DESFIRE_4K;
        case 8:
          return Type.MIFARE_DESFIRE_8K;
        default:
          return Type.UNKNOWN;
      }
    }
  }

  public CardInfo.Type type;

  /* card metadata, PAN for EMV_CARD, UID or NUID for other types */
  public String meta;

  CardInfo(CardInfo.Type type, String meta) {
    this.meta = meta;
    this.type = type;
  }
}