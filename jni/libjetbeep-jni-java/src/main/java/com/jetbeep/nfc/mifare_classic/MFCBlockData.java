package com.jetbeep.nfc.mifare_classic;

public class MFCBlockData {

  public static final int SIZE = 16;
  
  public int blockNo = 0;
  public byte[] value;

  public MFCBlockData(int blockNo, byte[] value) {
    this(blockNo);
    if (value.length != MFCBlockData.SIZE) {
      throw new Error("Invalid data size");
    }
    this.value = value;
  }

  public MFCBlockData(int blockNo) {
    if (blockNo < 0 || blockNo >= 256) {
      throw new Error("blockNo is out of range");
    }
    this.blockNo = blockNo;
    this.value = new byte[SIZE];
  }
}