package com.jetbeep.nfc.mifare_classic;

public class MFCKey {
  public enum Type {
    NONE,
    KEY_A,
    KEY_B;
  }

  public static final int SIZE = 6;

  public MFCKey.Type type;

  public byte[] value;

  MFCKey(MFCKey.Type type, byte[] value) {
    this.type = type;
    if (value.length != MFCKey.SIZE) {
      throw new Error("Invalid key size");
    }
    this.value = value;
  }
}