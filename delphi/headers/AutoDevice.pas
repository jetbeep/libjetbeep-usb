{ JetBeep device detection and communication }
unit AutoDevice;

interface

uses classes, System.SysUtils, AutoDeviceImport, JetBeepTypes, VersionChecker;

type
  { Barcode }
  TBarcode = record
    { Barcode value or mobile phone number }
    Barcode: string;
    { Barcode type, could\should be ignored }
    BarcodeType: Integer;
  end;

  { This handler is called during waitingForBarcodes state once the user taps JetBeep device }
  TBarcodesHandler = reference to procedure(barcodes: array of TBarcode);

  { This handler is called during waitingForPaymentToken state once the user taps JetBeep devices and confirm the payment in mobile application
    @param token payment token from the user that must be transferred to payment backend to continue the payment process. }
  TPaymentTokenHandler = reference to procedure(token: string);

  { This handler is called once mobile connection state is changed }
  TMobileConnectedHandler = reference to procedure(isConnected: Boolean);

  { This handler is called once state of a JetBeep device is changed }
  TStateHandler = reference to procedure(state: TJetBeepDeviceState);

  { Automatically detects JetBeep device plugged into USB and performs communication with it }
  TAutoDevice = class(TObject)
  private
    handle: TAutoDeviceHandle;
  public
    { This handler is called during waitingForBarcodes state once the user taps JetBeep device }
    barcodesHandler: TBarcodesHandler;

    { This handler is called during waitingForPaymentToken state once the user taps JetBeep devices and confirm the payment in mobile application
      @param token payment token from the user that must be transferred to payment backend to continue the payment process. }
    tokenHandler: TPaymentTokenHandler;

    { This handler is called once mobile connection state is changed }
    mobileConnectedHandler: TMobileConnectedHandler;

    { This handler is called once state of a JetBeep device is changed }
    stateHandler: TStateHandler;

    { @raises EJetBeepInvalidVersion Raised if jetbeep-*.dll version is not equal to *.pas version. IMPORTANT: Do not try to catch this exception as it means you're doing something wrong }
    constructor Create;

    destructor Destroy; override;

    { Starts detecting JetBeep devices in the system.

      Once the device will be found the state will be changed to firmwareVersionNotSupported or sessionClosed
      @raises EJetBeepInvalidState device is already started
      @raises EJetBeepIO system error }
    procedure Start;

    { Stops detecting JetBeep devices in the system.

      The state will be changed to invalid.
      @raises EJetBeepInvalidState device is already stopped
      @raises EJetBeepIO system error }
    procedure Stop;

    { Opens session. It means that the device is waiting for incoming Bluetooth connection

      The state will be changed to sessionOpened
      @raises EJetBeepInvalidState current state is not sessionClosed
      @raises EJetBeepIO system error }
    procedure OpenSession;

    { Closes session. Bluetooth connection (if exists) will be terminated. The device is no longer accepting new connections.

      The state will be changed to sessionClosed
      @raises EJetBeepInvalidState current state is not sessionOpened
      @raises EJetBeepIO system error }
    procedure CloseSession;

    { Requests barcodes. The device will be waiting for barcodes from the mobile phone.

      The state will be changed to waitingForBarcodes

      Once the barcodes will be received the stae will automatically change to: sessionOpened
      @raises EJetBeepInvalidState current state is not sessionOpened
      @raises EJetBeepIO system error }
    procedure RequestBarcodes;

    { Cancels requesting barcodes. The device will no longer be waiting for barcodes from the mobile phone.

      The state will be changed to sessionOpened
      @raises EJetBeepInvalidState current state is not waitingForBarcodes
      @raises EJetBeepIO system error }
    procedure CancelBarcodes;

    { Requests payment token from the mobile device.

      The state will be changed to waitingForPaymentToken

      Once the token will be received the state will be changed to sessionOpened

      @param amountInCoins amount in coins. E.g. for $1.05 it should be 105
      @param transactionId unique transaction id provided by the merchant
      @param cashierId unique cashier id. Use this field if transaction id could not be unique across different POS
      @param metadata additional information attached to the payment
      @raises EJetBeepInvalidState current state is not sessionOpened
      @raises EJetBeepIO system error }
    procedure CreatePaymentToken(amountInCoins: Cardinal; transactionId: String;
      cashierId: String; metadata: array of TMetadata);

    { Cancels pending payment token request
      The state will be changed to sessionOpened
      @raises EJetBeepInvalidState current state is not waitingForPaymentToken
      @raises EJetBeepIO system error
    }
    procedure CancelPayment;

    { Returns True if there is currently active bluetooth connection }
    function IsMobileConnected: Boolean;

    { Returns version of the firmware }
    function Version: String;

    { Returns device id }
    function DeviceId: Cardinal;

    { Returns current device state }
    function state: TJetBeepDeviceState;
  end;

implementation

procedure CBarcodesHandler(error: TCJetBeepError; barcodes: PJetBeepBarcode;
  barcodesSize: Integer; data: THandle); cdecl; forward;
procedure CTokenHandler(error: TCJetBeepError; token: PAnsiChar; data: THandle);
  cdecl; forward;
procedure CMobileConnectedHandler(isConnected: Boolean; data: THandle);
  cdecl; forward;
procedure CStateHandler(state: TJetBeepDeviceState; data: THandle);
  cdecl; forward;

constructor TAutoDevice.Create;
begin
  TVersionChecker.check;
  inherited;
  handle := jetbeep_autodevice_new;
  barcodesHandler := nil;
  tokenHandler := nil;
  mobileConnectedHandler := nil;
  stateHandler := nil;

  jetbeep_autodevice_set_state_callback(handle, CStateHandler, THandle(Self));
  jetbeep_autodevice_set_mobile_connected_callback(handle,
    CMobileConnectedHandler, THandle(Self));
end;

destructor TAutoDevice.Destroy;
begin
  jetbeep_autodevice_free(handle);
  inherited;
end;

procedure TAutoDevice.Start;
var
  error: TCJetBeepError;
begin
  error := jetbeep_autodevice_start(handle);
  case error of
    JETBEEP_NO_ERROR:
      Exit;
    JETBEEP_ERROR_INVALID_STATE:
      raise EJetBeepInvalidState.Create('Invalid device state');
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure TAutoDevice.Stop;
var
  error: TCJetBeepError;
begin
  error := jetbeep_autodevice_stop(handle);
  case error of
    JETBEEP_NO_ERROR:
      Exit;
    JETBEEP_ERROR_INVALID_STATE:
      raise EJetBeepInvalidState.Create('Invalid device state');
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure TAutoDevice.OpenSession;
var
  error: TCJetBeepError;
begin
  error := jetbeep_autodevice_open_session(handle);
  case error of
    JETBEEP_NO_ERROR:
      Exit;
    JETBEEP_ERROR_INVALID_STATE:
      raise EJetBeepInvalidState.Create('Invalid device state');
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure TAutoDevice.CloseSession;
var
  error: TCJetBeepError;
begin
  error := jetbeep_autodevice_close_session(handle);
  case error of
    JETBEEP_NO_ERROR:
      Exit;
    JETBEEP_ERROR_INVALID_STATE:
      raise EJetBeepInvalidState.Create('Invalid device state');
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure CBarcodesHandler(error: TCJetBeepError; barcodes: PJetBeepBarcode;
  barcodesSize: Integer; data: THandle);
var
  AutoDevice: TAutoDevice;
  convertedBarcodes: array of TBarcode;
  pbarcode: PJetBeepBarcode;
  i: Integer;
begin
  if error <> JETBEEP_NO_ERROR then
  begin
    Exit;
  end;

  if data = 0 then
  begin
    Exit;
  end;

  AutoDevice := TAutoDevice(data);
  SetLength(convertedBarcodes, barcodesSize);
  pbarcode := barcodes;
  for i := 0 to barcodesSize - 1 do
  begin
    convertedBarcodes[i].Barcode := string(pbarcode^.Barcode);
    convertedBarcodes[i].BarcodeType := pbarcode^.BarcodeType;
    Inc(pbarcode);
  end;

  if AutoDevice.barcodesHandler = nil then
  begin
    Exit;
  end;

  AutoDevice.barcodesHandler(convertedBarcodes);
end;

procedure TAutoDevice.RequestBarcodes;
var
  error: TCJetBeepError;
begin
  error := jetbeep_autodevice_request_barcodes(handle, CBarcodesHandler,
    THandle(Self));
  case error of
    JETBEEP_NO_ERROR:
      begin
        Exit;
      end;
    JETBEEP_ERROR_INVALID_STATE:
      raise EJetBeepInvalidState.Create('Invalid device state');
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure TAutoDevice.CancelBarcodes;
var
  error: TCJetBeepError;
begin
  error := jetbeep_autodevice_cancel_barcodes(handle);
  case error of
    JETBEEP_NO_ERROR:
      Exit;
    JETBEEP_ERROR_INVALID_STATE:
      raise EJetBeepInvalidState.Create('Invalid device state');
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure CTokenHandler(error: TCJetBeepError; token: PAnsiChar; data: THandle);
var
  AutoDevice: TAutoDevice;
  convertedToken: string;
begin
  if error <> JETBEEP_NO_ERROR then
  begin
    Exit;
  end;

  if data = 0 then
  begin
    Exit;
  end;

  AutoDevice := TAutoDevice(data);
  convertedToken := String(token);
  if AutoDevice.tokenHandler = nil then
  begin
    Exit;
  end;

  AutoDevice.tokenHandler(convertedToken);
end;

procedure TAutoDevice.CreatePaymentToken(amountInCoins: Cardinal;
  transactionId: String; cashierId: String; metadata: array of TMetadata);
var
  error: TCJetBeepError;
begin
  error := jetbeep_autodevice_create_payment_token(handle, amountInCoins,
    PAnsiChar(AnsiString(transactionId)), CTokenHandler, THandle(Self),
    PAnsiChar(AnsiString(cashierId)), @metadata[0], Length(metadata));
  case error of
    JETBEEP_NO_ERROR:
      begin
        Exit;
      end;
    JETBEEP_ERROR_INVALID_STATE:
      raise EJetBeepInvalidState.Create('Invalid device state');
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;

end;

procedure TAutoDevice.CancelPayment;
var
  error: TCJetBeepError;
begin
  error := jetbeep_autodevice_cancel_payment(handle);
  case error of
    JETBEEP_NO_ERROR:
      Exit;
    JETBEEP_ERROR_INVALID_STATE:
      raise EJetBeepInvalidState.Create('Invalid device state');
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

function TAutoDevice.IsMobileConnected: Boolean;
begin
  Result := jetbeep_autodevice_is_mobile_connected(handle);
end;

procedure CMobileConnectedHandler(isConnected: Boolean; data: THandle);
var
  AutoDevice: TAutoDevice;
begin
  AutoDevice := TAutoDevice(data);

  if AutoDevice.mobileConnectedHandler = nil then
  begin
    Exit;
  end;

  AutoDevice.mobileConnectedHandler(isConnected);
end;

function TAutoDevice.Version: String;
begin
  Result := String(jetbeep_autodevice_version(handle));
end;

function TAutoDevice.DeviceId: Cardinal;
begin
  Result := jetbeep_autodevice_device_id(handle);
end;

function TAutoDevice.state: TJetBeepDeviceState;
begin
  Result := jetbeep_autodevice_state(handle);
end;

procedure CStateHandler(state: TJetBeepDeviceState; data: THandle); cdecl;
var
  AutoDevice: TAutoDevice;
begin
  AutoDevice := TAutoDevice(data);
  if AutoDevice.stateHandler = nil then
  begin
    Exit;
  end;

  AutoDevice.stateHandler(state);
end;

end.
