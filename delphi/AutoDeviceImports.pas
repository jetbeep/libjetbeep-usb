unit AutoDeviceImports;

interface

uses
  SysUtils;

type
  AutoDeviceHandle = THandle;

function jetbeep_autodevice_new: AutoDeviceHandle; stdcall;

implementation

  const DLLName = 'jetbeep.dll';

function jetbeep_autodevice_new; external DLLName;

end.
