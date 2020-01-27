unit DeviceHandler;

interface

uses System.SysUtils, AutoDevice, JetBeepTypes;

type
  TDeviceHandler = class(TObject)
  public
    procedure BarcodesReceived(barcodes: array of TBarcode);
    procedure TokenReceived(token: String);
    procedure MobileConnected(isConnected: Boolean);
    function StateToString(state: TJetBeepDeviceState): String;
    procedure DeviceStateChanged(state: TJetBeepDeviceState);
  end;

implementation

procedure TDeviceHandler.BarcodesReceived(barcodes: array of TBarcode);
var
  barcode: TBarcode;
begin
  Writeln('Received: ', Length(barcodes), ' barcodes');
  for barcode in barcodes do
  begin
    Writeln('Value: ', barcode.Barcode)
  end;

end;

procedure TDeviceHandler.TokenReceived(token: String);
begin
  Writeln('Token: ', token);
end;

procedure TDeviceHandler.MobileConnected(isConnected: Boolean);
begin
  Writeln('Mobile connected: ', isConnected);
end;

function TDeviceHandler.StateToString(state: TJetBeepDeviceState): String;
begin
    case state of
       JETBEEP_STATE_INVALID:
        Result:= 'State: Invalid';
       JETBEEP_STATE_FIRMWARE_VERSION_NOT_SUPPORTED:
        Result:= 'State: firmware version is not supported';
       JETBEEP_STATE_SESSION_OPENED:
        Result:= 'State: session is opened';
       JETBEEP_STATE_SESSION_CLOSED:
        Result:= 'State: session is closed';
       JETBEEP_STATE_WAITING_FOR_BARCODES:
        Result:= 'State: waiting for barcodes';
       JETBEEP_STATE_WAITING_FOR_PAYMENT_RESULT:
        Result:= 'State: waiting for payment result';
       JETBEEP_STATE_WAITING_FOR_CONFIRMATION:
        Result:= 'State: waiting for confirmation';
       JETBEEP_STATE_WAITING_FOR_PAYMENT_TOKEN:
        Result:= 'State: waiting for paymen token';
       else
        Result:= 'Unknown state';
    end;
end;

procedure TDeviceHandler.DeviceStateChanged(state: TJetBeepDeviceState);
begin
  Writeln('State changed to: ', StateToString(state));
end;

end.
