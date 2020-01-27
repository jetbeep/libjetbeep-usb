program jetbeep_example;

{$APPTYPE CONSOLE}

{$R *.res}

uses
  System.SysUtils,
  AutoDeviceImport in 'headers\AutoDeviceImport.pas',
  JetbeepTypes in 'headers\JetbeepTypes.pas',
  AutoDevice in 'headers\AutoDevice.pas',
  DeviceHandler in 'DeviceHandler.pas';

var
  AutoDevice: TAutoDevice;
  DeviceHandler: TDeviceHandler;
  Metadata: array of TMetadata;
  Input: String;
begin
  AutoDevice:= TAutoDevice.Create;
  DeviceHandler:= TDeviceHandler.Create;
  Metadata := nil; // metadata is used only for partial payments as a key\value of parameters
  repeat
     ReadLn(Input);
     if Input = 'exit' then
        break;

     try
      if Input = 'start' then
        AutoDevice.Start
      else if Input = 'stop' then
        AutoDevice.Stop
      else if Input = 'opensession' then
        AutoDevice.OpenSession
      else if Input = 'closesession' then
        AutoDevice.CloseSession
      else if Input = 'requestbarcodes' then
        AutoDevice.RequestBarcodes(DeviceHandler.BarcodesReceived)
      else if Input = 'cancelbarcodes' then
        AutoDevice.CancelBarcodes
      else if Input = 'createpaymenttoken' then
        AutoDevice.CreatePaymentToken(100, 'YourTransactionId',
          'YourCashierIdOrEmptyString',
          Metadata,
          DeviceHandler.TokenReceived)
      else if Input = 'cancelpayment' then
        AutoDevice.CancelPayment
      else
        Writeln('Invalid command')
     except
     on E: Exception do
       Writeln(E.ClassName, ': ', E.Message);
     end;
  until False;

AutoDevice.Free;
DeviceHandler.Free;
end.
