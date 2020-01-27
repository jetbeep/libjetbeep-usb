unit AutoDevice;

interface

uses classes, System.SysUtils, AutoDeviceImport, JetBeepTypes;

type
    TBarcode = record
      Barcode: string;
      BarcodeType: Integer;
    end;

    TBarcodesHandler = reference to procedure (barcodes: array of TBarcode);

    TAutoDevice = class(TObject)
    private
        handle: TAutoDeviceHandle;
    public
        barcodesHandler: TBarcodesHandler;

        constructor Create;
        destructor Destroy; override;

        procedure Start;
        procedure Stop;
        procedure OpenSession;
        procedure CloseSession;
        procedure RequestBarcodes(handler: TBarcodesHandler);
end;

  procedure CBarcodesHandler(error: TJetBeepError; barcodes: PJetBeepBarcode; barcodesSize: Integer; data: THandle); cdecl;

implementation

constructor TAutoDevice.Create;
begin
    inherited;
    handle := jetbeep_autodevice_new;
    barcodesHandler := nil;
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
  barcode: TBarcode;
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
    barcodesHandler := handler;
    error:= jetbeep_autodevice_request_barcodes(handle, CBarcodesHandler, THandle(Self));
    case error of
      JETBEEP_NO_ERROR: Exit;
      JETBEEP_ERROR_INVALID_STATE: raise EJetBeepInvalidState.Create('Invalid device state');
      else raise EJetBeepIO.Create('System input-output exception');
    end;
end;

end.
