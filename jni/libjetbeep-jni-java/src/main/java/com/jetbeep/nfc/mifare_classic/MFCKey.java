package com.jetbeep.nfc.mifare_classic;

public class MFCKey {
  public enum Type {
    NONE (0),
    KEY_A(1),
    KEY_B(2);

    private final int value;
    private Type(int value) {
        this.value = value;
    }
    public int getValue() {
        return value;
    }
  }

  public static final int SIZE = 6;

  public MFCKey.Type type;

  public byte[] value;

  public MFCKey(MFCKey.Type type, byte[] value) {
    this.type = type;
    if (value.length != MFCKey.SIZE) {
      throw new Error("Invalid key size");
    }
    this.value = value;
  }
}