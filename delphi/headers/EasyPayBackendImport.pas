{ Internal unit, you should NEVER use it }
unit EasyPayBackendImport;

interface

uses
  SysUtils, JetBeepTypes;

type
  TEasyPayBackendHandle = THandle;

function jetbeep_easypay_new(environment: TEasyPayEnvironment;
  merchantKey: PAnsiChar): TEasyPayBackendHandle; cdecl;
procedure jetbeep_easypay_free(handle: TEasyPayBackendHandle); cdecl;
function jetbeep_easypay_make_payment(handle: TEasyPayBackendHandle;
  merchantTransactionId: PAnsiChar; paymentToken: PAnsiChar;
  amountInCoins: Cardinal; deviceId: Cardinal;
  callback: TEasyPayPaymentResultCallback; data: THandle; cashierId: PAnsiChar)
  : TCJetBeepError; cdecl;
function jetbeep_easypay_make_payment_partials(handle: TEasyPayBackendHandle;
  merchantTransactionId: PAnsiChar; paymentToken: PAnsiChar;
  amountInCoins: Cardinal; deviceId: Cardinal; metadata: PJetBeepMetadata;
  metadataSize: Integer; callback: TEasyPayPaymentResultCallback; data: THandle;
  cashierId: PAnsiChar): TCJetBeepError; cdecl;
function jetbeep_easypay_make_refund(handle: TEasyPayBackendHandle;
  easypayTransactionId: LongInt; amountInCoins: Cardinal; deviceId: Cardinal;
  callback: TEasyPayRefundResultCallback; data: THandle): TCJetBeepError; cdecl;
function jetbeep_easypay_make_refund_partials(handle: TEasyPayBackendHandle;
  easypayTransactionId: LongInt; amountInCoins: Cardinal; deviceId: Cardinal;
  callback: TEasyPayRefundResultCallback; data: THandle): TCJetBeepError; cdecl;

implementation

const
{$IFDEF WIN32}
  DLLName = 'jetbeep_i386.dll';
{$ELSEIF Defined(WIN64)}
  DLLName = 'jetbeep_x86_64.dll';
{$ELSE}
{$MESSAGE Fatal 'Platform not supported!'}
{$ENDIF}
function jetbeep_easypay_new; external DLLName;
procedure jetbeep_easypay_free; external DLLName;
function jetbeep_easypay_make_payment; external DLLName;
function jetbeep_easypay_make_payment_partials; external DLLName;
function jetbeep_easypay_make_refund; external DLLName;
function jetbeep_easypay_make_refund_partials; external DLLName;

end.
