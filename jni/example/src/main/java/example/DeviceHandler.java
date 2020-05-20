package example;

import com.jetbeep.*;
import com.jetbeep.nfc.*;
import com.jetbeep.nfc.mifare_classic.*;
import java.util.HashMap;


public class DeviceHandler extends AutoDevice {
  public EasyPayHandler backend;

  public DeviceHandler() {
    super();
    backend = new EasyPayHandler();
  }

  public String transactionId;
  public int amountInCoins;  
  public String cashierId;
  public HashMap<String, String> metadata;

  public void onBarcodes(Barcode[] barcodes) {
    System.out.println("received " + barcodes.length + " barcodes");
    for (int i = 0; i < barcodes.length; i++) {
      System.out.println(barcodes[i].value);
    }    
  }

  public void onPaymentToken(String token) {
    System.out.println("received token: " + token);
    if (metadata.size() > 0) {
      backend.makePaymentPartials(transactionId, token, amountInCoins, deviceId(), metadata, cashierId);
    } else {
      backend.makePayment(transactionId, token, amountInCoins, deviceId(), cashierId);
    }
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

  public void onNFCDetectionEvent(DetectionEvent event) {
    if (event.event == DetectionEvent.Event.DETECTED) {
      System.out.println("NFC: card detected:");
      switch(event.cardInfo.type) {
         case EMV_CARD:
            System.out.println("Bank card, PAN + exp. date: " + event.cardInfo.meta);
         break;
         case MIFARE_CLASSIC_1K://falls through
            System.out.println("MIFARE Classic 1K, UUID/NUID: " + event.cardInfo.meta + " memory 1024, blocks 64");
         break;
         case MIFARE_CLASSIC_4K:
            System.out.println("MIFARE Classic 4K, UUID/NUID: " + event.cardInfo.meta + "memory 4096, blocks 256");
         break;
         case MIFARE_PLUS_2K: //falls through
         case MIFARE_PLUS_4K:
            System.out.println("MIFARE PLUS family, UUID/NUID: " + event.cardInfo.meta);
         break;
         case MIFARE_DESFIRE_2K: //falls through
         case MIFARE_DESFIRE_4K:
         case MIFARE_DESFIRE_8K:
            System.out.println("MIFARE DESFIRE family");
         break;
         case UNKNOWN: //falls through
         default:
         System.out.println("unknown card type");
      }
      return;
    }
    if (event.event == DetectionEvent.Event.REMOVED) {
      System.out.println("NFC: card removed");
      return;
    }
  }

  public void onNFCDetectionError(DetectionError error) {
    switch(error) {
      case UNSUPPORTED: 
        System.out.println("NFC: detected card type is not supported");
      break;
      case MULTIPLE_CARDS: 
        System.out.println("NFC: There is multiple cards in detection field");
      break;
      case UNKNOWN: //falls through
      default:
      System.out.println("NFC: Unknown detection error");
    }
  }


  public void free() {
    backend.free();
    super.free();
  }
}