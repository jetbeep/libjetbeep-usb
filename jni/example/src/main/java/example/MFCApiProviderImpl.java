package example;

import com.jetbeep.*;
import com.jetbeep.nfc.*;
import com.jetbeep.nfc.mifare_classic.*;

public class MFCApiProviderImpl extends MFCApiProvider {

    public void onReadResult(MFCBlockData data, final Exception error) {
        if (error != null) {
            if (error instanceof MFCOperationException) {
               System.out.println("Mifare Classic read error reason: " + error.toString());
            } else {
               System.out.println(error.toString());
            }
            return;
        }
        System.out.println("MFCApiProvider read success: " + data.value.toString());
    }

    public void onWriteResult(final Exception error) {
        if (error != null) {
            if (error instanceof MFCOperationException) {
               System.out.println("Mifare Classic read error reason: " + error.toString());
            } else {
               System.out.println(error.toString());
            }
            return;
        }
        System.out.println("MFCApiProvider write success");
    }

    private MFCApiProviderImpl(long ptr) {
        super(ptr);
    }
}