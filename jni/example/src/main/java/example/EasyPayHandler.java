package example;

import com.jetbeep.*; 

class EasyPayHandler extends EasyPayBackend {
  EasyPayHandler() {
    super(EasyPayBackend.Environment.Production, "F09612A780C041D3939EE8C9CE8DC560");
  }

  public void onPaymentResult(PaymentResult result) {
    if (result.isSuccessful()) {
      System.out.println("payment successful, easyPayTransactionId: " + result.easyPayTransactionId + ", easyPayPaymentRequestUid: " + result.easyPaymentRequestUid);      
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