package com.jetbeep.nfc.mifare_classic;

public class MFCApiProvider {

  public MFCApiProvider(long native_ptr) {
    ptr = ptr;
  }

  /**
   * Frees resources in native library. DON'T FORGET to call once you don't need the object or it
   * wiil cause a memory leak!
   */
  public void free() {
    free(ptr);
  }

  /**
   * <p>This method is used to read block of Mifare card data.</p>
   * @param blockNo Mifare block number, starting from 0
   * @param MFCKey Mifare sector key, for sector where specified block contained
   * @return MFCBlockData for selected block of Mifare card
  */
  public MFCBlockData readBlock(int blockNo, MFCKey key) {
      return native_readBlock(ptr, blockNo, key);
  }

  /**
   * <p>This method is used to write block of Mifare card data.</p>
   * @param blockData data to write and block number starting from 0
   * @param MFCKey Mifare sector key, for sector where specified block contained
   * @return nothing
  */
  public void writeBlock(MFCBlockData blockData, MFCKey key) {
      native_writeBlock(ptr, blockData, key);
  }

  private native void free(long ptr);

  private native MFCBlockData native_readBlock(long ptr, int blockNo, MFCKey key);
  private native void native_writeBlock(long ptr, MFCBlockData blockData, MFCKey key);

  private long ptr;
}