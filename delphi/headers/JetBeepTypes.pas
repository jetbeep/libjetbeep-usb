unit JetBeepTypes;

interface

type

TJetBeepError = (
  JETBEEP_NO_ERROR = 0,
  JETBEEP_ERROR_INVALID_STATE = 1,
  JETBEEP_ERROR_IO = 2
);

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
  barcode: PChar;
  barcodeType: Integer;
end;

PJetBeepBarcodes = JetBeepBarcode;

TJetBeepBarcodesCallback = procedure(error: TJetBeepError; barcodes: PJetBeepBarcodes; barcodesSize: Integer); cdecl;
TJetBeepDeviceStateCallback = procedure(state: TJetBeepDeviceState); cdecl;

implementation

end.
