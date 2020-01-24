program jetbeep_example;

{$APPTYPE CONSOLE}

{$R *.res}

uses
  System.SysUtils,
  AutoDeviceImport in 'headers\AutoDeviceImport.pas';

var
  AutoDevice: AutoDeviceHandle;
begin
  try
    AutoDevice := jetbeep_autodevice_new();
  except
    on E: Exception do
      Writeln(E.ClassName, ': ', E.Message);
  end;
end.
