{ Internal unit, you should NEVER use it }
unit AutoDeviceImport;

interface

uses
  SysUtils, JetBeepTypes;

type
  TAutoDeviceHandle = THandle;

function jetbeep_autodevice_new: TAutoDeviceHandle; cdecl;
procedure jetbeep_autodevice_free(handle: TAutoDeviceHandle); cdecl;
function jetbeep_autodevice_start(handle: TAutoDeviceHandle)
  : TCJetBeepError; cdecl;
function jetbeep_autodevice_stop(handle: TAutoDeviceHandle)
  : TCJetBeepError; cdecl;
function jetbeep_autodevice_open_session(handle: TAutoDeviceHandle)
  : TCJetBeepError; cdecl;
function jetbeep_autodevice_close_session(handle: TAutoDeviceHandle)
  : TCJetBeepError; cdecl;
function jetbeep_autodevice_request_barcodes(handle: TAutoDeviceHandle;
  callback: TCJetBeepBarcodesCallback; data: THandle): TCJetBeepError; cdecl;
function jetbeep_autodevice_cancel_barcodes(handle: TAutoDeviceHandle)
  : TCJetBeepError; cdecl;
function jetbeep_autodevice_create_payment_token(handle: TAutoDeviceHandle;
  amountInCoins: Cardinal; tranasctionId: PAnsiChar;
  callback: TCJetBeepPaymentTokenCallback; data: THandle; cashierId: PAnsiChar;
  metadata: PJetBeepMetadata; metadataSize: Integer): TCJetBeepError; cdecl;
function jetbeep_autodevice_cancel_payment(handle: TAutoDeviceHandle)
  : TCJetBeepError; cdecl;
function jetbeep_autodevice_is_mobile_connected(handle: TAutoDeviceHandle)
  : Boolean; cdecl;
function jetbeep_autodevice_version(handle: TAutoDeviceHandle)
  : PAnsiChar; cdecl;
function jetbeep_autodevice_device_id(handle: TAutoDeviceHandle)
  : Cardinal; cdecl;
function jetbeep_autodevice_state(handle: TAutoDeviceHandle)
  : TJetBeepDeviceState; cdecl;
procedure jetbeep_autodevice_set_state_callback(handle: TAutoDeviceHandle;
  callback: TCJetBeepDeviceStateCallback; data: THandle); cdecl;
procedure jetbeep_autodevice_set_mobile_connected_callback
  (handle: TAutoDeviceHandle; callback: TCJetBeepMobileConnectedCallback;
  data: THandle); cdecl;

implementation

const
{$IFDEF WIN32}
  DLLName = 'jetbeep_i386.dll';
{$ELSEIF Defined(WIN64)}
  DLLName = 'jetbeep_x86_64.dll';
{$ELSE}
{$MESSAGE Fatal 'Platform not supported!'}
{$ENDIF}
function jetbeep_autodevice_new; external DLLName;
procedure jetbeep_autodevice_free; external DLLName;
function jetbeep_autodevice_start; external DLLName;
function jetbeep_autodevice_stop; external DLLName;
function jetbeep_autodevice_open_session; external DLLName;
function jetbeep_autodevice_close_session; external DLLName;
function jetbeep_autodevice_request_barcodes; external DLLName;
function jetbeep_autodevice_cancel_barcodes; external DLLName;
function jetbeep_autodevice_create_payment_token; external DLLName;
function jetbeep_autodevice_cancel_payment; external DLLName;
function jetbeep_autodevice_is_mobile_connected; external DLLName;
function jetbeep_autodevice_version; external DLLName;
function jetbeep_autodevice_device_id; external DLLName;
function jetbeep_autodevice_state; external DLLName;
procedure jetbeep_autodevice_set_state_callback; external DLLName;
procedure jetbeep_autodevice_set_mobile_connected_callback; external DLLName;

end.
