package example;

import java.io.BufferedReader;
import java.lang.System.Logger;
import java.util.HashMap;
import java.util.Scanner;

import com.jetbeep.*;

import example.DeviceHandler;
class Main {
  public static void main(String[] args) {        
    System.out.println("example started");
    com.jetbeep.Logger.setCoutEnabled(true);
    com.jetbeep.Logger.setLogLevel(com.jetbeep.Logger.Level.verbose);    
    DeviceHandler handler = new DeviceHandler();
    Scanner scanner = new Scanner(System.in);

    loop: while (true) {
      String input;
      try {
        input = scanner.nextLine().toLowerCase();
      } catch (Exception e) {
        e.printStackTrace();
        break loop;
      }
      String[] splitted = input.split(" ");
      if (splitted.length < 1) {
        System.out.println("invalid input");
        continue;
      }
      String cmd = splitted[0];      

      switch (cmd) {
        case "exit":          
          break loop;
        case "start":
          try {
            handler.start();
          } catch (Exception e) {
            e.printStackTrace();
          }
          break;
        case "stop":
          try {
            handler.stop();
          } catch (Exception e) {
            e.printStackTrace();
          }
          break;
        case "opensession":
        case "open_session":
          try {
            handler.openSession();
          } catch (Exception e) {
            e.printStackTrace();
          }
          break;
        case "closesession":
        case "close_session":
          try {
            handler.closeSession();
          } catch (Exception e) {
            e.printStackTrace();
          }
          break;
        case "requestbarcodes":
        case "request_barcodes":
          try {
            handler.requestBarcodes();
          } catch (Exception e) {
            e.printStackTrace();
          }
          break;
        case "cancelbarcodes":
        case "cancel_barcodes":
          try {
            handler.cancelBarcodes();
          } catch (Exception e) {
            e.printStackTrace();
          }
          break;
        case "createpaymenttoken":
        case "create_payment_token":
          if (splitted.length < 3 || splitted.length > 5) {
            System.out.println("invalid parameter count");
            System.out.println("usage: create_payment_token amount:Int transactionId:String cashierId:String metadata:[String:String]");
            System.out.println("%cashierId% %metadata% - are optional fields");
            break;
          }           
          
          int amount = 0;
          try {
            amount = Integer.parseInt(splitted[1]);
          } catch (Exception e) {
            System.out.println("unable to parse amount");
            break;
          }
          String transactionId = splitted[2];
          String cashierId = "";
          HashMap<String, String> metadata = new HashMap<>();
          if (splitted.length > 3) {
            cashierId = splitted[3];
          }

          if (splitted.length > 4) {
            String[] splittedMetadata = splitted[4].split(";");
            for (int i = 0; i < splittedMetadata.length; ++i) {
              String[] keyValue = splittedMetadata[i].split(":");
              if (keyValue.length != 2) {
                System.out.println("invalid metadata format");
                System.out.println("metadata - key1:value1;key2:value2");
                break;
              }
              metadata.put(keyValue[0], keyValue[1]);
            }          
          }
          try {
            handler.createPaymentToken(amount, transactionId, cashierId, metadata);
            handler.amountInCoins = amount;
            handler.transactionId = transactionId;
            handler.cashierId = cashierId;
          } catch (Exception e) {
            e.printStackTrace();
          }          
          break;
        case "cancelpayment":
        case "cancel_payment":
          try {
            handler.cancelPayment();
          } catch (Exception e) {
            e.printStackTrace();
          }
          break;
        case "refund":
          if (splitted.length != 2) {
            System.out.println("invalid params count");
            break;
          }
          
          try {
            long easyPayTransactionId = Long.parseLong(splitted[1]);
            handler.backend.makeRefund(easyPayTransactionId, handler.amountInCoins, handler.deviceId());
          } catch (Exception e) {
            e.printStackTrace();
          }
          break;
        default:
          System.out.println("unknown command: " + cmd);
          break;
      }
    }

    scanner.close();
    handler.free(); // IMPORTANT: it's required to free handler, otherwise there will be a memory-leak
  }
}