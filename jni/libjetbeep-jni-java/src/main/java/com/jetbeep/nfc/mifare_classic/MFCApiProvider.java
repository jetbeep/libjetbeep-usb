package com.jetbeep.nfc.mifare_classic;

public abstract class MFCApiProvider {
   /** 
   * <p>This callback will be fired once the data is read from Mifare card.</p>
   * @param blockData data from requested block
   */
  abstract public void onReadDone(MFCBlockData blockData);

  /** 
   * <p>This callback will be fired once write operation is completed</p>
   */
  abstract public void onWriteDone();

  public void readBlock(int blockNo, MFCKey key) {
      //TODO
  }

  public void writeBlock(MFCBlockData blockData, MFCKey key) {
    //TODO
  }
}