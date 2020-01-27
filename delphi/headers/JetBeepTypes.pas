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

TEasyPayEnvironment = (
  EASYPAY_BACKEND_DEVELOPMENT = 0,
  EASYPAY_BACKEND_PRODUCTION = 1
);

TJetBeepBarcode = record
  barcode: PAnsiChar;
  barcodeType: Integer;
end;

PJetBeepBarcode = ^TJetBeepBarcode;

TJetBeepMetadata = record
  key: PAnsiChar;
  value: PAnsiChar
end;

PJetBeepMetadata = ^TJetBeepMetadata;

TEasyPayPaymentResult = record
  errorString: PAnsiChar;
  easyPayTransactionId: Integer;
  easyPayPaymentRequestUid: PAnsiChar;
end;

TEasyPayRefundResult = record
  errorString: PAnsiChar;
end;

TJetBeepBarcodesCallback = procedure(error: TJetBeepError; barcodes: PJetBeepBarcode; barcodesSize: Integer; data: THandle); cdecl;
TJetBeepPaymentTokenCallback = procedure(error: TJetBeepError; token: PAnsiChar; data: THandle); cdecl;
TJetBeepDeviceStateCallback = procedure(state: TJetBeepDeviceState; data: THandle); cdecl;
TJetBeepMobileConnectedCallback = procedure(connected: Boolean; data: THandle); cdecl;
TEasyPayPaymentResultCallback = procedure(result: TEasyPayPaymentResult); cdecl;
TEasyPayRefundResultCallback = procedure(result: TEasyPayRefundResult); cdecl;


implementation

end.
