package example;

import com.jetbeep.*; 

class EasyPayHandler extends EasyPayBackend {
  EasyPayHandler() {
    super(EasyPayBackend.Environment.Production, "here_should_be_merchant_token");
  }

  public void onPaymentResult(PaymentResult result) {
    if (result.isSuccessful()) {
      System.out.println("payment successful, easyPayTransactionId: " + result.easyPayTransactionId);      
    } else {
      System.out.println("payment error: " + result.errorString);
    }
  }

  public void onRefundResult(RefundResult result) {
    if (result.isSuccessful()) {
      System.out.println("refund successful");
    } else {
      System.out.println("refund error: " + result.errorString);
    }    
  }
}