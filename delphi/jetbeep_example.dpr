program jetbeep_example;

{$APPTYPE CONSOLE}
{$R *.res}

uses
  System.SysUtils,
  Windows,
  AutoDeviceImport in 'headers\AutoDeviceImport.pas',
  JetBeepTypes in 'headers\JetBeepTypes.pas',
  AutoDevice in 'headers\AutoDevice.pas',
  DeviceHandler in 'DeviceHandler.pas',
  EasyPayBackendImport in 'headers\EasyPayBackendImport.pas',
  EasyPayBackend in 'headers\EasyPayBackend.pas',
  LoggerImport in 'headers\LoggerImport.pas',
  Logger in 'headers\Logger.pas',
  VersionChecker in 'headers\VersionChecker.pas';

var
  AutoDevice: TAutoDevice;
  DeviceHandler: TDeviceHandler;
  EasyPayBackend: TEasyPayBackend;
  Metadata: array of TMetadata;
  Input: String;
  AmountInCoins: Cardinal;

begin
  SetConsoleOutputCP(CP_UTF8);
  // uncomment this if you need additional logs
  TLogger.externalOutputEnabled := true;
  TLogger.level := JETBEEP_LOGGER_DEBUG;
  TLogger.loggerLineOutputHandler := DeviceHandler.LogLine;
  AmountInCoins := 100;
  AutoDevice := TAutoDevice.Create;
  EasyPayBackend := TEasyPayBackend.Create(EASYPAY_BACKEND_DEVELOPMENT,
    'your-merchant-key');

  DeviceHandler := TDeviceHandler.Create;

  AutoDevice.barcodesHandler := DeviceHandler.BarcodesReceived;
  AutoDevice.tokenHandler := DeviceHandler.TokenReceived;
  AutoDevice.mobileConnectedHandler := DeviceHandler.MobileConnected;
  AutoDevice.stateHandler := DeviceHandler.DeviceStateChanged;

  EasyPayBackend.paymentResultHandler := DeviceHandler.PaymentResultReceived;
  EasyPayBackend.refundResultHandler := DeviceHandler.RefundResultReceived;
  Metadata := nil;
  // metadata is used only for partial payments as a key\value of parameters
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
        AutoDevice.RequestBarcodes
      else if Input = 'cancelbarcodes' then
        AutoDevice.CancelBarcodes
      else if Input = 'createpaymenttoken' then
        AutoDevice.CreatePaymentToken(AmountInCoins, 'YourTransactionId',
          'YourCashierIdOrEmptyString', Metadata)
      else if Input = 'mobileconnected' then
        Writeln('Mobile connected: ', AutoDevice.IsMobileConnected)
      else if Input = 'cancelpayment' then
        AutoDevice.CancelPayment
      else if Input = 'makepayment' then
        EasyPayBackend.MakePayment('YourTransactionId', DeviceHandler.token,
          AmountInCoins, AutoDevice.DeviceId, 'YourCashierIdOrEmptyString')
      else if Input = 'makerefund' then
        EasyPayBackend.MakeRefund(DeviceHandler.easyPayTransactionId,
          AmountInCoins, AutoDevice.DeviceId)
      else if Input = 'version' then
        Writeln('Device version: ', AutoDevice.Version)
      else if Input = 'deviceid' then
        Writeln('Device id: ', AutoDevice.DeviceId)
      else if Input = 'state' then
        Writeln(DeviceHandler.StateToString(AutoDevice.State))
      else
        Writeln('Invalid command')
    except
      on E: Exception do
        Writeln(E.ClassName, ': ', E.Message);
    end;
  until False;

  AutoDevice.Free;
  EasyPayBackend.Free;
  DeviceHandler.Free;

end.
