unit DeviceHandler;

interface

uses System.SysUtils, AutoDevice;

type
  TDeviceHandler = class(TObject)
  public
    procedure BarcodesReceived(barcodes: array of TBarcode);
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

end.
