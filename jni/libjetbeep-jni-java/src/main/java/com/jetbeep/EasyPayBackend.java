package com.jetbeep;

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

    public boolean isSuccessful() {
      return errorString == null;
    }

    PaymentResult(String errorString, long easyPayTransactionId) {
      this.errorString = errorString;
      this.easyPayTransactionId = easyPayTransactionId;
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
    
  private void onNativePaymentResult(String errorString, long easyPayTransactionId) {
    PaymentResult result = new PaymentResult(errorString, easyPayTransactionId);
    onPaymentResult(result);
  }

  private void onNativeRefundResult(String errorString) {
    RefundResult result = new RefundResult(errorString);
    onRefundResult(result);
  }
  
  private long ptr;
  private native long init(int env, String merchantToken);
  private native void makePayment(long ptr, String transactionId, String token, int amountInCoins, long deviceId, String cashierId);
  private native void makeRefund(long ptr, long easyPayTransactionId, int amountInCoins, long deviceId);
  private native void free(long ptr);
}