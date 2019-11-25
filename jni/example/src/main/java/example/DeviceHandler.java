package example;

import com.jetbeep.*;

public class DeviceHandler extends AutoDevice {
  public EasyPayHandler backend;

  public DeviceHandler() {
    super();
    backend = new EasyPayHandler();
  }

  public String transactionId;
  public int amountInCoins;  
  public String cashierId;

  public void onBarcodes(Barcode[] barcodes) {
    System.out.println("received " + barcodes.length + " barcodes");
    for (int i = 0; i < barcodes.length; i++) {
      System.out.println(barcodes[i].value);
    }    
  }

  public void onPaymentToken(String token) {
    System.out.println("received token: " + token);
    backend.makePayment(transactionId, token, amountInCoins, deviceId(), cashierId);
  }

  public void onStateChange(AutoDevice.State newState) {
    System.out.println("state changed to: " + newState.toString());

    if (newState == AutoDevice.State.sessionClosed) {
      System.out.println("deviceId: " + deviceId());
      System.out.println("version: " + version());
    }
  }

  public void onMobileConnectionChange(boolean isConnected) {
    System.out.println("is mobile connected == " + isConnected);
  }

  public void free() {
    backend.free();
    super.free();
  }
}