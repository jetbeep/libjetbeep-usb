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
begin
  AutoDevice:= TAutoDevice.Create;
  DeviceHandler:= TDeviceHandler.Create;
  try
    AutoDevice.Start;
    ReadLn;
    AutoDevice.OpenSession;
    ReadLn;
    AutoDevice.RequestBarcodes(DeviceHandler.BarcodesReceived);
    ReadLn;
    AutoDevice.CloseSession;
  except
    on E: Exception do
      Writeln(E.ClassName, ': ', E.Message);
  end;
  DeviceHandler.Free;
  AutoDevice.Free;
  ReadLn;
end.
