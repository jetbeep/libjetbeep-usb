package com.jetbeep.nfc.mifare_classic;

import java.lang.IllegalStateException;
import java.io.IOException;

abstract public class MFCApiProvider {

  protected MFCApiProvider(long native_ptr) {
    saveObj(native_ptr);
    ptr = native_ptr;
  }

  /**
   * Frees resources in native library. DON'T FORGET to call once you don't need the object or it
   * wiil cause a memory leak!
   */
  public void free() {
    free(ptr);
  }

  /**
  * <p>This callback will be fired once writeBlock operation is completed </p>
  * @param data - block content in case of success, null in case of error.
  * @param error - exception (MFCOperationException) instance if any error occurred during operation, null in case of success.
  */
  abstract public void onReadResult(MFCBlockData data, final Exception error);

  /**
  * <p>This callback will be fired once readBlock operation is completed </p>
  * @param error - exception instance if any error occurred during operation, null in case of success.
  */
  abstract public void onWriteResult(final Exception error);

  /**
   * <p>This method is used to read block of Mifare card data. Will trigger onReadResult.</p>
   * @param blockNo Mifare block number, starting from 0
   * @param key Mifare sector key, for sector where specified block contained
   * @throws IllegalStateException if no NFC card is detected
   * @throws IOException in case of system error
  */
  public void readBlock(int blockNo, MFCKey key) throws MFCOperationException, IllegalStateException, IOException {
      native_readBlock(ptr, blockNo, key);
  }

  /**
   * <p>This method is used to write block of Mifare card data. Will trigger onWriteResult.</p>
   * @param blockData data to write and block number starting from 0
   * @param key Mifare sector key, for sector where specified block contained
   * @throws IllegalStateException if no NFC card is detected
   * @throws IOException in case of system error
  */
  public void writeBlock(MFCBlockData blockData, MFCKey key) throws MFCOperationException, IllegalStateException, IOException {
      native_writeBlock(ptr, blockData, key);
  }

  private native void free(long ptr);
  private native void saveObj(long ptr);

  private native void native_readBlock(long ptr, int blockNo, MFCKey key);
  private native void native_writeBlock(long ptr, MFCBlockData blockData, MFCKey key);

  private long ptr;
}