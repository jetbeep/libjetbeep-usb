unit AutoDevice;

interface

uses classes, System.SysUtils, AutoDeviceImport, JetBeepTypes;

type
  TBarcode = record
    Barcode: string;
    BarcodeType: Integer;
  end;

  TBarcodesHandler = reference to procedure(barcodes: array of TBarcode);
  TPaymentTokenHandler = reference to procedure(token: string);
  TMobileConnectedHandler = reference to procedure(isConnected: Boolean);
  TStateHandler = reference to procedure(state: TJetBeepDeviceState);

  TAutoDevice = class(TObject)
  private
    handle: TAutoDeviceHandle;
  public
    barcodesHandler: TBarcodesHandler;
    tokenHandler: TPaymentTokenHandler;
    mobileConnectedHandler: TMobileConnectedHandler;
    stateHandler: TStateHandler;

    constructor Create;
    destructor Destroy; override;

    procedure Start;
    procedure Stop;
    procedure OpenSession;
    procedure CloseSession;
    procedure RequestBarcodes;
    procedure CancelBarcodes;
    procedure CreatePaymentToken(amountInCoins: Cardinal; transactionId: String;
      cashierId: String; metadata: array of TMetadata);
    procedure CancelPayment;
    function IsMobileConnected: Boolean;
    function Version: String;
    function DeviceId: Cardinal;
    function state: TJetBeepDeviceState;
  end;

procedure CBarcodesHandler(error: TCJetBeepError; barcodes: PJetBeepBarcode;
  barcodesSize: Integer; data: THandle); cdecl;
procedure CTokenHandler(error: TCJetBeepError; token: PAnsiChar;
  data: THandle); cdecl;
procedure CMobileConnectedHandler(isConnected: Boolean; data: THandle); cdecl;
procedure CStateHandler(state: TJetBeepDeviceState; data: THandle); cdecl;

implementation

constructor TAutoDevice.Create;
begin
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
