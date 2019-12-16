package com.jetbeep;

import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;

abstract public class EasyPayBackend {
  static {
    Library.loadAndCheckVersion();
  }
  /**
   * <p>Server environment</p>
   */
  public enum Environment {
    /**
     * Development server (fake payments)
     */
    Development(0),
    /**
     * Production server (real payments)
     */
    Production(1);

    Environment(int value) {
      this.value = value;
    }

    private int value;
  }

  public class PaymentResult {    
    /**
     * Error string. If it's null then the payment was successful
     */
    public String errorString;
    /**
     * Unique transaction id of the payment in the EasyPay processing.
     */
    public long easyPayTransactionId;

    /**
     * UUID of the payment request in the EasyPay processing (used for partials payment refund)
     */
    public String easyPaymentRequestUid;

    public boolean isSuccessful() {
      return errorString == null;
    }

    PaymentResult(String errorString, long easyPayTransactionId, String easyPaymentRequestUid) {
      this.errorString = errorString;
      this.easyPayTransactionId = easyPayTransactionId;
      this.easyPaymentRequestUid = easyPaymentRequestUid;
    }

    PaymentResult(String errorString) {
      this.errorString = errorString;
    }
  }

  public class RefundResult {
    /**
     * Error string. If it's null then the payment was successful
     */    
    public String errorString;

    public boolean isSuccessful() {
      return errorString == null;
    }

    RefundResult(String errorString) {
      this.errorString = errorString;
    }
  }

  
  /** 
   * Constructs EasyPayBackend object
   * @param environment environment, see beelo
   * @param merchantToken merchant's credentials
   * @see Environment
   */
  public EasyPayBackend(Environment environment, String merchantToken) {
    ptr = init(environment.value, merchantToken);
  }

  
  /** 
   * <p>Performs payment to EasyPay backend. The result will be returned </p>
   * @param transactionId unique POS transaction id (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param token payment token received from AutoDevice
   * @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param deviceId current deviceId. Could be obtained as AutoDevice.deviceId()
   */
  public void makePayment(String transactionId, String token, int amountInCoins, long deviceId) {
    makePayment(transactionId, token, amountInCoins, deviceId, "");
  }

  
  /** 
   * <p>Performs payment to EasyPay backend. The result will be returned </p>
   * @param transactionId unique POS transaction id (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param token payment token received from AutoDevice
   * @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param deviceId current deviceId. Could be obtained as AutoDevice.deviceId()
   * @param cashierId unique POS id (should be the same value as provided to AutoDevice.createPaymentToken)
   */
  public void makePayment(String transactionId, String token, int amountInCoins, long deviceId, String cashierId) {
    makePayment(ptr, transactionId, token, amountInCoins, deviceId, cashierId);
  }

    /** 
   * <p>Performs Partials payment to EasyPay backend. The result will be returned </p>
   * @param transactionId unique POS transaction id (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param token payment token received from AutoDevice
   * @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param metadata additional information attached to the payment (payment partials recepients)
   * @param deviceId current deviceId. Could be obtained as AutoDevice.deviceId()
   */
  public void makePaymentPartials(String transactionId, String token, int amountInCoins, long deviceId, HashMap<String, String> metadata) {
    makePaymentPartials(transactionId, token, amountInCoins, deviceId, metadata, "");
  }

  
  /** 
   * <p>Performs Partials payment to EasyPay backend. The result will be returned </p>
   * @param transactionId unique POS transaction id (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param token payment token received from AutoDevice
   * @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param deviceId current deviceId. Could be obtained as AutoDevice.deviceId()
   * @param metadata additional information attached to the payment (payment partials recepients)
   * @param cashierId unique POS id (should be the same value as provided to AutoDevice.createPaymentToken)
   */
  public void makePaymentPartials(String transactionId, String token, int amountInCoins, long deviceId,
      HashMap<String, String> metadata, String cashierId) {
    String[] keysMetadata = new String[metadata.size()];
    String[] valuesMetadata = new String[metadata.size()];
    int index = 0;

    for (Map.Entry<String, String> e : metadata.entrySet()) {
      keysMetadata[index] = e.getKey();
      valuesMetadata[index] = e.getValue();
      ++index;
    }
    makePaymentPartials(ptr, transactionId, token, amountInCoins, deviceId, keysMetadata, valuesMetadata, cashierId);
  }
  
  /** 
   * <p>Result received from the EasyPay backend</p>
   * @param result result of payment, see PaymentResult
   * @see PaymentResult
   */
  abstract public void onPaymentResult(PaymentResult result);

  
  /** 
   * <p>Perform refund operation to EasyPay backend</p>
   * @param easyPayTransactionId transaction id received in onPaymentResult
   * @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param deviceId current deviceId. Could be obtained as AutoDevice.deviceId()
   */
  public void makeRefund(long easyPayTransactionId, int amountInCoins, long deviceId) {
    makeRefund(ptr, easyPayTransactionId, amountInCoins, deviceId);
  }

  /** 
   * <p>Perform refund operation for Partials payment to EasyPay backend</p>
   * @param paymentRequestUid PaymentRequestUid received in onPaymentResult
   * @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to AutoDevice.createPaymentToken)
   * @param deviceId current deviceId. Could be obtained as AutoDevice.deviceId()
   */
  public void makeRefundPartials(String paymentRequestUid, int amountInCoins, long deviceId) {
    makeRefundPartials(ptr, paymentRequestUid, amountInCoins, deviceId);
  }

  
  /** 
   * <p>Result received from the EasyPay backend</p>
   * @param result result of refund, see RefundResult
   * @see RefundResult
   */
  abstract public void onRefundResult(RefundResult result);

  /**
   * <p>Frees native resource. DON'T FORGET to call it once you don't need the object</p>
   */
  public void free() {
    free(ptr);
    ptr = 0;
  }
    
  private void onNativePaymentResult(String errorString, long easyPayTransactionId, String easyPayPaymentRequestUid) {
    PaymentResult result = new PaymentResult(errorString, easyPayTransactionId, easyPayPaymentRequestUid);
    onPaymentResult(result);
  }

  private void onNativeRefundResult(String errorString) {
    RefundResult result = new RefundResult(errorString);
    onRefundResult(result);
  }
  
  private long ptr;

  private native long init(int env, String merchantToken);

  private native void makePayment(long ptr, String transactionId, String token, int amountInCoins, long deviceId,
      String cashierId);

  private native void makePaymentPartials(long ptr, String transactionId, String token, int amountInCoins,
      long deviceId, String[] metadataKeys, String[] metadataValues, String cashierId);

  private native void makeRefund(long ptr, long easyPayTransactionId, int amountInCoins, long deviceId);

  private native void makeRefundPartials(long ptr, String paymentRequestUid, int amountInCoins, long deviceId);

  private native void free(long ptr);
}