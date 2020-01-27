unit AutoDevice;

interface

uses classes, System.SysUtils, AutoDeviceImport, JetBeepTypes;

type
    TBarcode = record
      Barcode: string;
      BarcodeType: Integer;
    end;

    TMetadata = record
      Key: string;
      Value: string;
    end;

    TBarcodesHandler = reference to procedure (barcodes: array of TBarcode);
    TPaymentTokenHandler = reference to procedure (token: string);
    TMobileConnectedHandler = reference to procedure (isConnected: Boolean);
    TStateHandler = reference to procedure (state: TJetBeepDeviceState);

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
        procedure RequestBarcodes(handler: TBarcodesHandler);
        procedure CancelBarcodes;
        procedure CreatePaymentToken(amountInCoins: Cardinal; transactionId: String; cashierId: String; metadata: array of TMetadata; handler: TPaymentTokenHandler);
        procedure CancelPayment;
        function IsMobileConnected: Boolean;
        function Version: String;
        function DeviceId: Cardinal;
        function State: TJetBeepDeviceState;

        procedure SetMobileConnectedHandler(handler: TMobileConnectedHandler);
        procedure SetStateHandler(handler: TStateHandler);
end;

  procedure CBarcodesHandler(error: TJetBeepError; barcodes: PJetBeepBarcode; barcodesSize: Integer; data: THandle); cdecl;
  procedure CTokenHandler(error: TJetBeepError; token: PAnsiChar; data: THandle); cdecl;
  procedure CMobileConnectedHandler(isConnected: Boolean; data: THandle); cdecl;
  procedure CStateHandler(state: TJetBeepDeviceState; data: THandle); cdecl;
implementation

constructor TAutoDevice.Create;
begin
    inherited;
    handle := jetbeep_autodevice_new;
    barcodesHandler := nil;
    tokenHandler:= nil;
    mobileConnectedHandler:= nil;
    stateHandler:= nil;
end;

destructor TAutoDevice.Destroy;
begin
    jetbeep_autodevice_free(handle);
    inherited;
end;

procedure TAutoDevice.Start;
var
    error: TJetBeepError;
begin
    error:= jetbeep_autodevice_start(handle);
    case error of
      JETBEEP_NO_ERROR: Exit;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
    end;
end;

procedure TAutoDevice.Stop;
var
    error: TJetBeepError;
begin
    error:= jetbeep_autodevice_stop(handle);
    case error of
      JETBEEP_NO_ERROR: Exit;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
    end;
end;

procedure TAutoDevice.OpenSession;
var
    error: TJetBeepError;
begin
    error:= jetbeep_autodevice_open_session(handle);
    case error of
      JETBEEP_NO_ERROR: Exit;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
    end;
end;

procedure TAutoDevice.CloseSession;
var
    error: TJetBeepError;
begin
    error:= jetbeep_autodevice_close_session(handle);
    case error of
      JETBEEP_NO_ERROR: Exit;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
    end;
end;

procedure CBarcodesHandler(error: TJetBeepError; barcodes: PJetBeepBarcode; barcodesSize: Integer; data: THandle);
var
  autoDevice: TAutoDevice;
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

  autoDevice:= TAutoDevice(data);
  SetLength(convertedBarcodes, barcodesSize);
  pbarcode:= barcodes;
  for i := 0 to barcodesSize - 1 do
  begin
    convertedBarcodes[i].Barcode := string(pbarcode^.barcode);
    convertedBarcodes[i].BarcodeType := pbarcode^.barcodeType;
    Inc(pbarcode);
  end;

  autoDevice.barcodesHandler(convertedBarcodes);
end;

procedure TAutoDevice.RequestBarcodes(handler: TBarcodesHandler);
var
    error: TJetBeepError;
begin
    error:= jetbeep_autodevice_request_barcodes(handle, CBarcodesHandler, THandle(Self));
    case error of
      JETBEEP_NO_ERROR:
      begin
        barcodesHandler := handler;
        Exit;
      end;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
    end;
end;

procedure TAutoDevice.CancelBarcodes;
var
    error: TJetBeepError;
begin
    error:= jetbeep_autodevice_cancel_barcodes(handle);
    case error of
      JETBEEP_NO_ERROR: Exit;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
    end;
end;

procedure CTokenHandler(error: TJetBeepError; token: PAnsiChar; data: THandle);
var
  autoDevice: TAutoDevice;
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

  autoDevice:= TAutoDevice(data);
  convertedToken := String(token);
  autoDevice.tokenHandler(convertedToken);
end;

procedure TAutoDevice.CreatePaymentToken(amountInCoins: Cardinal;
  transactionId: String;
  cashierId: String;
  metadata: array of TMetadata;
  handler: TPaymentTokenHandler);
var
  error: TJetBeepError;
begin
  error:= jetbeep_autodevice_create_payment_token(handle,
    amountInCoins,
    PAnsiChar(AnsiString(transactionId)),
    CTokenHandler, THandle(Self),
    PAnsiChar(AnsiString(cashierId)),
    @metadata[0],
    Length(metadata));
  case error of
      JETBEEP_NO_ERROR:
      begin
        tokenHandler:= handler;
        Exit;
      end;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
  end;

end;

procedure TAutoDevice.CancelPayment;
var
    error: TJetBeepError;
begin
    error:= jetbeep_autodevice_cancel_payment(handle);
    case error of
      JETBEEP_NO_ERROR: Exit;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
    end;
end;

function TAutoDevice.IsMobileConnected: Boolean;
begin
   Result:= jetbeep_autodevice_is_mobile_connected(handle);
end;

procedure CMobileConnectedHandler(isConnected: Boolean; data: THandle);
var
  autoDevice: TAutoDevice;
begin
  autoDevice:= TAutoDevice(data);

  autoDevice.mobileConnectedHandler(isConnected);
end;

procedure TAutoDevice.SetMobileConnectedHandler(handler: TMobileConnectedHandler);
begin
  jetbeep_autodevice_set_mobile_connected_callback(handle, CMobileConnectedHandler, THandle(Self));
  mobileConnectedHandler := handler;
end;

function TAutoDevice.Version: String;
begin
  Result := String(jetbeep_autodevice_version(handle));
end;

function TAutoDevice.DeviceId: Cardinal;
begin
  Result := jetbeep_autodevice_device_id(handle);
end;

function TAutoDevice.State: TJetBeepDeviceState;
begin
  Result := jetbeep_autodevice_state(handle);
end;

procedure CStateHandler(state: TJetBeepDeviceState; data: THandle); cdecl;
var
  autoDevice: TAutoDevice;
begin
  autoDevice:= TAutoDevice(data);

  autoDevice.stateHandler(state);
end;

procedure TAutoDevice.SetStateHandler(handler: TStateHandler);
begin
  jetbeep_autodevice_set_state_callback(handle, CStateHandler, THandle(Self));
  stateHandler:= handler;
end;

end.
