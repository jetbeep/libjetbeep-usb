package com.jetbeep;

abstract public class EasyPayBackend {
  static {
    Library.loadAndCheckVersion();
  }
    
  public enum Environment {
    Development(0),
    Production(1);

    Environment(int value) {
      this.value = value;
    }

    private int value;
  }

  public class PaymentResult {    
    public String errorString;
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
    public String errorString;

    public boolean isSuccessful() {
      return errorString == null;
    }

    RefundResult(String errorString) {
      this.errorString = errorString;
    }
  }

  public EasyPayBackend(Environment environment, String merchantToken) {
    ptr = init(environment.value, merchantToken);
  }

  public void makePayment(String transactionId, String token, int amountInCoins, long deviceId) {
    makePayment(transactionId, token, amountInCoins, deviceId, "");
  }

  public void makePayment(String transactionId, String token, int amountInCoins, long deviceId, String cashierId) {
    makePayment(ptr, transactionId, token, amountInCoins, deviceId, cashierId);
  }

  abstract public void onPaymentResult(PaymentResult result);

  public void makeRefund(long easyPayTransactionId, int amountInCoins, long deviceId) {
    makeRefund(ptr, easyPayTransactionId, amountInCoins, deviceId);
  }

  abstract public void onRefundResult(RefundResult result);

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