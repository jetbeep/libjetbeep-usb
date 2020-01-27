unit JetBeepTypes;

interface
uses SysUtils;

type

TJetBeepError = (
  JETBEEP_NO_ERROR = 0,
  JETBEEP_ERROR_INVALID_STATE = 1,
  JETBEEP_ERROR_IO = 2
);

EJetBeepInvalidState = class(Exception);
EJetBeepIO = class(Exception);

TJetBeepDeviceState = (
  JETBEEP_STATE_INVALID = 0, 
  JETBEEP_STATE_FIRMWARE_VERSION_NOT_SUPPORTED,

  JETBEEP_STATE_SESSION_OPENED,
  JETBEEP_STATE_SESSION_CLOSED,
  JETBEEP_STATE_WAITING_FOR_BARCODES,

  JETBEEP_STATE_WAITING_FOR_PAYMENT_RESULT,
  JETBEEP_STATE_WAITING_FOR_CONFIRMATION,

  JETBEEP_STATE_WAITING_FOR_PAYMENT_TOKEN
);

JetBeepBarcode = record
  barcode: PAnsiChar;
  barcodeType: Integer;
end;

PJetBeepBarcode = ^JetBeepBarcode;

JetBeepMetadata = record
  key: PAnsiChar;
  value: PAnsiChar
end;

PJetBeepMetadata = ^JetBeepMetadata;

TJetBeepBarcodesCallback = procedure(error: TJetBeepError; barcodes: PJetBeepBarcode; barcodesSize: Integer; data: THandle); cdecl;
TJetBeepPaymentTokenCallback = procedure(error: TJetBeepError; token: PAnsiChar; data: THandle); cdecl;
TJetBeepDeviceStateCallback = procedure(state: TJetBeepDeviceState; data: THandle); cdecl;
TJetBeepMobileConnectedCallback = procedure(connected: Boolean; data: THandle); cdecl;



implementation

end.
