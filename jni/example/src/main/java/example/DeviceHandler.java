package example;

import com.jetbeep.*;

public class DeviceHandler extends AutoDevice {
  public void onBarcodes(Barcode[] barcodes) {
    System.out.println("received " + barcodes.length + " barcodes");
    for (int i = 0; i < barcodes.length; i++) {
      System.out.println(barcodes[i].value);
    }    
  }

  public void onPaymentToken(String token) {
    System.out.println("received token: " + token);
  }

  public void onStateChange(AutoDevice.State newState) {
    System.out.println("state changed to: " + newState.toString());
  }
}