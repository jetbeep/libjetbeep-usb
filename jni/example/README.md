

## Running example project

Requirements: Java Runtime

Linux and Mac OS:

```bash
cd bin
./example
```

Windows:

```
cd bin
./example.bat
```

Avaliable commands:

* ```start``` - starts detection of JetBeep devices. Once the device will be plugged into your system's USB port, it will automatically detected by the application.
* ```stop``` - stops detection of JetBeep devices. If there was any opened device, it will be automatically closed
* ```open_session``` - tells the JetBeep device to accept incoming Bluetooth connection from the mobile application
* ```close_session``` - terminates active Bluetooth connection (if it exists) and tells the JetBeep device to stop accepting Bluetooth connections
* ```request_barcodes``` - requests barcodes information from the mobile application (once the Bluetooth connection will be established)
* ```cancel_barcodes``` - cancels pending "requests_barcodes" command
* ```create_payment_token %amount% %transaction_id% %cashierId% %metadata%``` - requests payment token from the mobile application. Parameters:
  * ```%amount%``` - amount of the transaction specified in coins. E.g. for $1.05 it should be 105
  * ```%transaction_id%``` - (String). Unique identifier of the transaction
  * ```%cashier_id%``` - (String, Optional). Identifier of the cashier. This field could be used to distinguish transactions in case if different cashiers could produce same transaction_ids. 
  * ```%metadata%``` - (String, Optional). Additional fields for the transaction in key-value format. Example: ```key1:value1;key2:value2;```
* ```cancel_payment``` - cancels pending "create_payment_token" command

Source code of the example project: [Main.java](https://todo)

## Integrating libjetbeep-jni-java into your project

1. Copy ```libjetbeep-jni.so```, ```libjetbeep-jni.dylib``` or ```jetbeep-jni.dll``` library file from ```libjetbeep-jni``` folder to:
    * Either to one of your Java PATH locations, e.g. "/usr/local/lib"
    * Or to your Java executable folder
    * Or to any folder you like (in this case you have to add this folder to -Djava.library.path argument of the Java process)
1. Copy ```lib\libjetbeep-jni-java-*.*.*.jar``` to your project and add it to classpath. Example for gradle build system:

```
dependencies {
    implementation files('lib/libjetbeep-jni-java-*.*.*.jar')
}
```