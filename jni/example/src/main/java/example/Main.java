package example;

import java.io.BufferedReader;
import java.util.Scanner;

import com.jetbeep.*;

import example.DeviceHandler;
class Main {
  public static void main(String[] args) {        
    System.out.println("example started");
    DeviceHandler handler = new DeviceHandler();
    Scanner scanner = new Scanner(System.in);

    loop: while (true) {
      String cmd;
      try {
        cmd = scanner.nextLine().toLowerCase();
      } catch (Exception e) {
        e.printStackTrace();
        break loop;
      }
      

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
        default:
          System.out.println("unknown command: " + cmd);
          break;
      }
    }

    scanner.close();
    handler.free(); // IMPORTANT: it's required to free handler, otherwise there will be a memory-leak
  }
}