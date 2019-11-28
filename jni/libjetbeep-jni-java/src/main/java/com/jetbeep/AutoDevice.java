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
     * <p>Possible meanings: </p>
     * <ul> 
     * <li>Device is not plugged via USB</li>
     * <li>Device is not detected by the OS</li>
     * <li>Error occured on the device. The library automatically attempts to reset and recover</li>
     * </ul>
     * 
     * <p>Can be changed to: </p>
     * 
     * <ul>
     * <li>firmwareVersionNotSupported</li>
     * <li>sessionClosed</li>
     * </ul>
     */
    invalid,
    /**
     * <p>Possible meanings: </p>
     * <ul>
     * <li>The device was successfully detected, but its firmware version is outdated. Please update!</li>
     * </ul>
     * 
     * <p>Can be changed to: </p>
     * <ul>
     * <li>invalid</li>
     * </ul>
     */
    firmwareVersionNotSupported,
    /**
     * <p>Possible meanings: </p>
     * <ul>
     * <li>Device is ready to work (recovered from invalid state)</li>
     * <li>Previous session was closed</li>
     * </ul>
     * 
     * <p>During this state the JetBeep device IS NOT accepting any incoming Bluetooth connections</p>
     * 
     * <p>Can be changed to: </p>
     * <ul>
     * <li>sessionOpened</li>
     * <li>invalid</li>
     * </ul>
     */
    sessionClosed,
    /**
     * <p>Possible meanings: </p>
     * <ul>
     * <li>Device is waiting for incoming Bluetooth connections</li>
     * </ul>
     * 
     * <p>During this state the JetBeep device IS accepting any incoming Bluetooth connections</p>
     * 
     * <p>Can be changed to: </p>
     * <ul>
     * <li>sessionClosed</li>
     * <li>waitingForBarcodes</li>
     * <li>waitingForPaymentToken</li>
     * <li>invalid</li>
     * </ul>
     */
    sessionOpened, 
    /**
     * <p>Possible meanings: </p>
     * <ul>
     * <li>Device is waiting for barcodes to be received from the mobile device</li>
     * </ul>
     * 
     * <p>Can be changed to: </p>
     * <ul>
     * <li>sessionOpened</li>
     * <li>invalid</li>
     * </ul>
     */
    waitingForBarcodes,
    /**
     * <p>Possible meanings: </p>
     * <ul>
     * <li>Device is waiting for payment token to be received from the mobile device</li>
     * </ul>
     * 
     * <p>Can be changed to: </p>
     * <ul>
     * <li>sessionOpened</li>
     * <li>invalid</li>
     * </ul>
     */
    waitingForPaymentToken,  
    /**
     * Can not be fired by the library. This constant is used for compatibility.
     */
    waitingForPaymentResult,
    /**
     * Can not be fired by the library. This constant is used for compatibility.
     */    
    waitingForConfirmation

  }

  
  /** 
   * Constructs AutoDevice. DON'T FORGET to call free() once you don't need the object or it
   * wiil cause a memory leak!
   */
  public AutoDevice() {
    ptr = init();
  }

  /**
   * Frees resources in native library. DON'T FORGET to call once you don't need the object or it
   * wiil cause a memory leak!
   */
  public void free() {
    free(ptr);
  }  

  
  /** 
   * <p>Starts detecting JetBeep devices in the system. </p>
   * <p>Once the device will be found the state will be changed to firmwareVersionNotSupported or sessionClosed</p>
   *    
   * @throws IllegalStateException device is already started
   * @throws IOException system error
   */
  public void start() throws IllegalStateException, IOException {
    start(ptr);
  }

  
  /** 
   * <p>Starts detecting JetBeep devices in the system. </p>
   * <p>The state will be changed to invalid.</p>
   * 
   * @throws IllegalStateException device is already stopped
   * @throws IOException system error
   */
  public void stop() throws IllegalStateException, IOException {
    stop(ptr);
  }

  
  /** 
   * <p>Opens session. It means that the device is waiting for incoming Bluetooth connection</p>
   * <p>The state will be changed to sessionOpened</p>
   * 
   * @throws IllegalStateException current state is not sessionClosed
   * @throws IOException system error
   */
  public void openSession() throws IllegalStateException, IOException {
    openSession(ptr);
  }

  
  /** 
   * <p>Closes session. Bluetooth connection (if exists) will be terminated. The device is no longer
   * accepting new connections.</p>
   * <p>The state will be changed to sessionClosed</p>
   * 
   * @throws IllegalStateException current state is not sessionOpened
   * @throws IOException system error
   */
  public void closeSession() throws IllegalStateException, IOException {
    closeSession(ptr);
  }

  
  /** 
   * <p>Requests barcodes. The device will be waiting for barcodes from the mobile phone.</p>
   * <p>The state will be changed to waitingForBarcodes</p>
   * <p>Once the barcodes will be received the stae will automatically change to: sessionOpened</p>
   * 
   * @throws IllegalStateException current state is not sessionOpened
   * @throws IOException system error
   */
  public void requestBarcodes() throws IllegalStateException, IOException {
    requestBarcodes(ptr);
  }

  
  /** 
   * <p>Cancels requesting barcodes. The device will no longer be waiting for barcodes from the mobile phone.</p>
   * <p>The state will be changed to sessionOpened</p>
   * 
   * @throws IllegalStateException current state is not waitingForBarcodes
   * @throws IOException system error
   */
  public void cancelBarcodes() throws IllegalStateException, IOException {
    cancelBarcodes(ptr);
  }

  
  /** 
   * <p>Requests payment token from the mobile device.</p>
   * <p>The state will be changed to waitingForPaymentToken</p>
   * <p>Once the token will be received the state will be changed to sessionOpened</p>
   * 
   * @param amount amount in coins. E.g. for $1.05 it should be 105
   * @param transactionId unique transaction id provided by the merchant
   * @param cashierId unique cashier id. Use this field if transaction id could not be unique across different POS
   * @param metadata additional information attached to the payment
   * @throws IllegalStateException current state is not sessionOpened
   * @throws IOException system error
   */
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

  
  /** 
   * <p>Requests payment token from the mobile device.</p>
   * <p>The state will be changed to waitingForPaymentToken</p>
   * <p>Once the token will be received the state will be changed to sessionOpened</p>
   * 
   * @param amount amount in coins. E.g. for $1.05 it should be 105
   * @param transactionId unique transaction id provided by the merchant
   * @param cashierId unique cashier id. Use this field if transaction id could not be unique across different POS
   * @throws IllegalStateException current state is not sessionOpened
   * @throws IOException system error
   */
  public void createPaymentToken(int amount, String transactionId, String cashierId)
    throws IllegalStateException, IOException {
    createPaymentToken(amount, transactionId, cashierId, new HashMap<String, String>());
  }

  
  /** 
   * <p>Requests payment token from the mobile device.</p>
   * <p>The state will be changed to waitingForPaymentToken</p>
   * <p>Once the token will be received the state will be changed to sessionOpened</p>
   * 
   * @param amount amount in coins. E.g. for $1.05 it should be 105
   * @param transactionId unique transaction id provided by the merchant
   * @throws IllegalStateException current state is not sessionOpened
   * @throws IOException system error
   */
  public void createPaymentToken(int amount, String transactionId)
    throws IllegalStateException, IOException {
    createPaymentToken(amount, transactionId, new String(""), new HashMap<String, String>());
  }

  
  /** 
   * <p>Cancels pending payment token request</p>
   * <p>The state will be changed to sessionOpened</p>
   * 
   * @throws IllegalStateException current state is not waitingForPaymentToken
   * @throws IOException system error
   */
  public void cancelPayment() throws IllegalStateException, IOException {
    cancelPayment(ptr);
  }

  
  /** 
   * Current state
   * @return State current state
   */
  public AutoDevice.State state() {
    return state(ptr);
  }

  
  /** 
   * <p>This field will be resolved once the device will be detected(sessionClosed)</p>
   * @return long unique identifier of the device
   */
  public long deviceId() {
    return deviceId(ptr);
  }

  
  /** 
   * <p>This field will be resolved once the device will be detected(sessionClosed)</p>
   * @return String current firmware version of the device
   */
  public String version() {
    return version(ptr);
  }

  
  /** 
   * @return boolean true - there is active Bluetooth connection to mobile device, false - otherwise
   */
  public boolean isMobileConnected() {
    return isMobileConnected(ptr);    
  }

  
  /** 
   * <p>This callback will be fired once the barcodes(loyalty) will be received from the mobile phone.</p>
   * <p>State will be changed to sessionOpened</p>
   * @param barcodes list of received barcodes
   */
  abstract public void onBarcodes(Barcode[] barcodes);
  
  /** 
   * <p>This callback will be fired once the payment token will be received from the mobile phone.</p>
   * <p>State will be changed to sessionOpened</p>
   * @param token payment token
   */
  abstract public void onPaymentToken(String token);
  
  /** 
   * <p>This callback will be fired once the state changes</p>
   * @param newState new state of the device
   */
  abstract public void onStateChange(State newState);
  
  /** 
   * <p>This callback will be fired once the mobile connection changes</p>
   * @param isConnected true - mobile phone connected, false - otherwise
   */
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