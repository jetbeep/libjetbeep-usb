package com.jetbeep.nfc.mifare_classic;

public class MFCOperationException extends Exception {

   String code;

   MFCOperationException(String code) {
      super("MFCOperationException code:" + code);
      this.code = code;
   }

   public String toString() {
      switch (this.code) {
         case "UNKNOWN":
            return "Mifare Unknown error";
         case "AUTH_ERROR":
            return "Invalid Mifare sector key";
         case "CARD_REMOVED":
            return "Card removed before operation completed";
         case "UNSUPPORTED_CARD_TYPE":
            return "Card type is not Mifare Classic";
         case "DATA_SIZE":
            return "Invalid content data size";
         case "INTERRUPTED":
            return "IO command interrupted";
         case "KEY_PARAM_INVALID":
            return "Invalid key format";
         case "PARAMS_INVALID":
            return "Block # is out of bounds";
      }
      return ("MFCOperationException code:" + code);
   }
}
