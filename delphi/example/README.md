## Folder structure:

* `doc` - HTML documentation of Units and Classes
* `example-project` - example command-line utility project
* * `example-project\jetbeep.groupproj` just open this file in Embarcadero RAD Studio and Run the project
* `example-project\jetbeep_i386.dll` and `example-project\jetbeep_x86_64.dll` - 32 and 64 bit version of libjetbeep 
* `bin` - pre-built command-line binaries (same as produced by jetbeep.groupproj)

## Running example project

Requirements: 

* Embarcadero RAD Studio (tested on Delphi 10.3)

Avaliable commands in command-line example:

* `start` - starts detection of JetBeep devices. Once the device will be plugged into your system's USB port, it will automatically detected by the application.
* `stop` - stops detection of JetBeep devices. If there was any opened device, it will be automatically closed
* `opensession` - tells the JetBeep device to accept incoming Bluetooth connection from the mobile application
* `closesession` - terminates active Bluetooth connection (if it exists) and tells the JetBeep device to stop accepting Bluetooth connections
* `requestbarcodes` - requests barcodes information from the mobile application (once the Bluetooth connection will be established)
* `cancelbarcodes` - cancels pending "requests_barcodes" command
* `createpaymenttoken` - requests payment token from the mobile application. 
* `cancelpayment` - cancels pending "create_payment_token" command
* `makepayment` - performs payment operation to EasyPay backend
* `makerefund` - performs refund operation to EasyPay backend
* `version` - prints device firmware version
* `deviceid` - prints device id

## Integrating into your project

1. Copy all *.pas files in `example-project\headers` to your Delphi project
2. Copy `jetbeep_i386.dll` and\or `jetbeep_x86_64.dll` to the root folder of your Delphi project
3. In Embarcadero RAD Studio open `Project->Options->Build Events->Post-build events->Commands` paste the following command:

for 32-bit:
`copy $(PROJECTDIR)\jetbeep_i386.dll $(OUTPUTDIR)\jetbeep_i386.dll`

for 64-bit:
`copy $(PROJECTDIR)\jetbeep_x86_64.dll $(OUTPUTDIR)\jetbeep_x86_64.dll`

Or alternatively:
Manually copy `jetbeep_i386.dll` to `Win32\Debug` `Win32\Release` and\or `jetbeep_x86_64.dll` to `Win64\Debug` `Win64\Release`

## Deployment of your binary
Just ensure that `jetbeep_i386.dll` or `jetbeep_x86_64.dll` is located in the same folder as your binary.