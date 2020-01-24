unit AutoDeviceImport;

interface
uses
  SysUtils;

type
  AutoDeviceHandle = THandle;

function jetbeep_autodevice_new: AutoDeviceHandle; cdecl;

implementation

const
{$IFDEF WIN32}
  DLLName = 'jetbeep_i386.dll';
{$ELSEIF Defined(WIN64)}
  DLLName = 'jetbeep_x86_64.dll';
{$ELSE}
  {$Message Fatal 'Platform not supported!'}
{$ENDIF}

  function jetbeep_autodevice_new; external DLLName;
end.
