unit JetBeepTypes;

interface
uses SysUtils;

type

TCJetBeepError = (
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

TCJetBeepBarcode = record
  barcode: PAnsiChar;
  barcodeType: Integer;
end;

PJetBeepBarcode = ^TCJetBeepBarcode;

TCJetBeepMetadata = record
  key: PAnsiChar;
  value: PAnsiChar
end;

PJetBeepMetadata = ^TCJetBeepMetadata;

TCEasyPayPaymentResult = record
  errorString: PAnsiChar;
  easyPayTransactionId: Integer;
  easyPayPaymentRequestUid: PAnsiChar;
end;

TCEasyPayRefundResult = record
  errorString: PAnsiChar;
end;

TCJetBeepBarcodesCallback = procedure(error: TCJetBeepError; barcodes: PJetBeepBarcode; barcodesSize: Integer; data: THandle); cdecl;
TCJetBeepPaymentTokenCallback = procedure(error: TCJetBeepError; token: PAnsiChar; data: THandle); cdecl;
TCJetBeepDeviceStateCallback = procedure(state: TJetBeepDeviceState; data: THandle); cdecl;
TCJetBeepMobileConnectedCallback = procedure(connected: Boolean; data: THandle); cdecl;
TEasyPayPaymentResultCallback = procedure(result: TCEasyPayPaymentResult); cdecl;
TEasyPayRefundResultCallback = procedure(result: TCEasyPayRefundResult); cdecl;


implementation

end.
