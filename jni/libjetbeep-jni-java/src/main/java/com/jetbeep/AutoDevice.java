package com.jetbeep;

import java.util.Map;

import com.jetbeep.Barcode;

import java.lang.IllegalStateException;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;

abstract public class AutoDevice {
  static {
    Library.loadAndCheckVersion();
  }

  public enum State {
    /**
     * 
     */
    invalid,
    firmwareVersionNotSupported,
    sessionOpened,
    sessionClosed,
    waitingForBarcodes,
  
    waitingForPaymentResult,
    waitingForConfirmation,
  
    waitingForPaymentToken
  }

  public AutoDevice() {
    ptr = init();
  }

  public void free() {
    free(ptr);
    ptr = 0;
  }  

  public void start() throws IllegalStateException, IOException {
    start(ptr);
  }

  public void stop() throws IllegalStateException, IOException {
    stop(ptr);
  }

  public void openSession() throws IllegalStateException, IOException {
    openSession(ptr);
  }

  public void closeSession() throws IllegalStateException, IOException {
    closeSession(ptr);
  }

  public void requestBarcodes() throws IllegalStateException, IOException {
    requestBarcodes(ptr);
  }

  public void cancelBarcodes() throws IllegalStateException, IOException {
    cancelBarcodes(ptr);
  }

  public void createPaymentToken(int amount, String transactionId, 
    String cashierId, HashMap<String, String> metadata) 
    throws IllegalStateException, IOException {    
    String[] keysMetadata = new String[metadata.size()];
    String[] valuesMetadata = new String[metadata.size()];
    int index = 0;
    
    for (Map.Entry<String, String> e: metadata.entrySet()) {
      keysMetadata[index] = e.getKey();
      valuesMetadata[index] = e.getValue();
      ++index;    
    }

    createPaymentToken(ptr, amount, transactionId, cashierId, keysMetadata, valuesMetadata);
  }

  public void createPaymentToken(int amount, String transactionId, String cashierId)
    throws IllegalStateException, IOException {
    createPaymentToken(amount, transactionId, cashierId, new HashMap<String, String>());
  }

  public void createPaymentToken(int amount, String transactionId)
    throws IllegalStateException, IOException {
    createPaymentToken(amount, transactionId, new String(""), new HashMap<String, String>());
  }

  public void cancelPayment() throws IllegalStateException, IOException {
    cancelPayment(ptr);
  }

  public AutoDevice.State state() {
    return state(ptr);
  }

  public long deviceId() {
    return deviceId(ptr);
  }

  public String version() {
    return version(ptr);
  }

  public boolean isMobileConnected() {
    return isMobileConnected(ptr);    
  }

  abstract public void onBarcodes(Barcode[] barcodes);
  abstract public void onPaymentToken(String token);
  abstract public void onStateChange(State newState);
  abstract public void onMobileConnectionChange(boolean isConnected);

  private void onBarcodeBegin(int size) {
    m_barcodes = new Barcode[size];
  }

  private void onBarcodeValue(int index, String cvalue, int ctype) {
    Barcode.Type type = Barcode.Type.fromInt(ctype);
    Barcode barcode = new Barcode(cvalue, type);
    m_barcodes[index] = barcode;
  }

  private void onBarcodeEnd() {
    onBarcodes(m_barcodes);
    m_barcodes = null;
  }

  private Barcode[] m_barcodes;

  private native long init();  
  private native void free(long ptr);

  private native void start(long ptr);
  private native void stop(long ptr);  
  private native void openSession(long ptr);
  private native void closeSession(long ptr);
  private native void requestBarcodes(long ptr);
  private native void cancelBarcodes(long ptr);
  private native void createPaymentToken(long ptr, int amount, String transactionId, 
    String cashierId, String[] metadataKeys, String[] metadataValues);
  private native void cancelPayment(long ptr);
  private native AutoDevice.State state(long ptr);
  private native boolean isMobileConnected(long ptr);
  private native long deviceId(long ptr);
  private native String version(long ptr);


  private long ptr;
}