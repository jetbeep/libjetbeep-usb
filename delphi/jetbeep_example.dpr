program jetbeep_example;

{$APPTYPE CONSOLE}

{$R *.res}

uses
  System.SysUtils,
  AutoDeviceImport in 'headers\AutoDeviceImport.pas',
  JetbeepTypes in 'headers\JetbeepTypes.pas';

var
  AutoDevice: TAutoDeviceHandle;
begin
  try
    AutoDevice := jetbeep_autodevice_new();
    jetbeep_autodevice_start(AutoDevice);
    ReadLn;
    jetbeep_autodevice_open_session(AutoDevice);
    ReadLn;
  except
    on E: Exception do
      Writeln(E.ClassName, ': ', E.Message);
  end;
end.
