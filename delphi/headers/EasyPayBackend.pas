{ Payments and refunds to EasyPay backend }
unit EasyPayBackend;

interface

uses classes, System.SysUtils, EasyPayBackendImport, JetBeepTypes,
  VersionChecker;

type
  { Result of the payment operation }
  TEasyPayPaymentResult = record
    { Error string. If it's nil then the payment was successful }
    errorString: String;
    { Unique EasyPay transaction id }
    easyPayTransactionId: Integer;
    { Unique EasyPay payment request id }
    easyPayPaymentRequestUid: String;
  end;

  { Result of the refund operation }
  TEasyPayRefundResult = record
    { Error string. If it's nil then the refund was successful }
    errorString: String;
  end;

  { This handler is called once payment result is received }
  TEasyPayPaymentResultHandler = reference to procedure
    (result: TEasyPayPaymentResult);

  { This handler is called once refund result is received }
  TEasyPayRefundResultHandler = reference to procedure
    (result: TEasyPayRefundResult);

  { HTTPS calls to EasyPay API }
  TEasyPayBackend = class(TObject)
  private
    handle: TEasyPayBackendHandle;
  public
    { This handler is called once payment result is received }
    paymentResultHandler: TEasyPayPaymentResultHandler;

    { This handler is called once refund result is received }
    refundResultHandler: TEasyPayRefundResultHandler;

    { @param environment dev or production server environment
      @param merchantToken merchant's credentials
      @raises EJetBeepInvalidVersion Raised if jetbeep-*.dll version is not equal to *.pas version. IMPORTANT: Do not try to catch this exception as it means you're doing something wrong }
    constructor Create(environment: TEasyPayEnvironment; merchantKey: String);
    destructor Destroy; override;

    { Performs payment to EasyPay backend
      @param transactionId unique POS transaction id (should be the same value as provided to TAutoDevice.createPaymentToken)
      @param token payment token received from TAutoDevice
      @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to TAutoDevice.createPaymentToken)
      @param deviceId current deviceId. Could be obtained as TAutoDevice.deviceId
      @param cashierId unique POS id (should be the same value as provided to AutoDevice.createPaymentToken)
    }
    procedure MakePayment(merchantTransactionId: String; paymentToken: String;
      amountInCoins: Cardinal; deviceId: Cardinal; cashierId: String);

    { Performs Partials payment to EasyPay backend.
      @param transactionId unique POS transaction id (should be the same value as provided to AutoDevice.createPaymentToken)
      @param token payment token received from TAutoDevice
      @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to TAutoDevice.createPaymentToken)
      @param metadata additional information attached to the payment (payment partials recepients)
      @param deviceId current deviceId. Could be obtained as TAutoDevice.deviceId
      @param cashierId unique POS id (should be the same value as provided to TAutoDevice.createPaymentToken)
    }
    procedure MakePaymentPartials(merchantTransactionId: String;
      paymentToken: String; amountInCoins: Cardinal; deviceId: Cardinal;
      metadata: array of TMetadata; cashierId: String);

    { Performs refund operation to EasyPay backend
      @param easyPayTransactionId transaction id received in paymentResultHandler
      @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to TAutoDevice.createPaymentToken)
      @param deviceId current deviceId. Could be obtained as TAutoDevice.deviceId()
    }
    procedure MakeRefund(easyPayTransactionId: LongInt; amountInCoins: Cardinal;
      deviceId: Cardinal);

    { Perform refund operation for Partials payment to EasyPay backend</p>
      @param paymentRequestUid PaymentRequestUid received in paymentResultHandler
      @param amountInCoins amount in coins. E.g. for $1.05 should be 105. (should be the same value as provided to TAutoDevice.createPaymentToken)
      @param deviceId current deviceId. Could be obtained as TAutoDevice.deviceId() }
    procedure MakeRefundPartials(easyPayTransactionId: LongInt;
      amountInCoins: Cardinal; deviceId: Cardinal);
  end;

implementation

procedure CPaymentResultHandler(result: TCEasyPayPaymentResult; data: THandle);
  cdecl; forward;
procedure CRefundResultHandler(result: TCEasyPayRefundResult; data: THandle);
  cdecl; forward;

procedure CPaymentResultHandler(result: TCEasyPayPaymentResult; data: THandle);
var
  EasyPayBackend: TEasyPayBackend;
  ConvertedResult: TEasyPayPaymentResult;
begin
  EasyPayBackend := TEasyPayBackend(data);

  if EasyPayBackend.paymentResultHandler = nil then
  begin
    Exit;
  end;

  ConvertedResult.errorString := System.Utf8ToUnicodeString(result.errorString);
  ConvertedResult.easyPayTransactionId := result.easyPayTransactionId;
  ConvertedResult.easyPayPaymentRequestUid :=
    String(result.easyPayPaymentRequestUid);
  EasyPayBackend.paymentResultHandler(ConvertedResult);
end;

procedure CRefundResultHandler(result: TCEasyPayRefundResult; data: THandle);
var
  EasyPayBackend: TEasyPayBackend;
  ConvertedResult: TEasyPayRefundResult;
begin
  EasyPayBackend := TEasyPayBackend(data);

  if EasyPayBackend.paymentResultHandler = nil then
  begin
    Exit;
  end;

  ConvertedResult.errorString := System.Utf8ToUnicodeString(result.errorString);
  EasyPayBackend.refundResultHandler(ConvertedResult);
end;

constructor TEasyPayBackend.Create(environment: TEasyPayEnvironment;
  merchantKey: String);
begin
  TVersionChecker.check;
  inherited Create;
  handle := jetbeep_easypay_new(environment,
    PAnsiChar(AnsiString(merchantKey)));
end;

destructor TEasyPayBackend.Destroy;
begin
  jetbeep_easypay_free(handle);
  inherited;
end;

procedure TEasyPayBackend.MakePayment(merchantTransactionId: String;
  paymentToken: String; amountInCoins: Cardinal; deviceId: Cardinal;
  cashierId: String);
var
  error: TCJetBeepError;
begin
  error := jetbeep_easypay_make_payment(handle,
    PAnsiChar(AnsiString(merchantTransactionId)),
    PAnsiChar(AnsiString(paymentToken)), amountInCoins, deviceId,
    CPaymentResultHandler, THandle(Self), PAnsiChar(AnsiString(cashierId)));

  case error of
    JETBEEP_NO_ERROR:
      Exit;
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure TEasyPayBackend.MakePaymentPartials(merchantTransactionId: String;
  paymentToken: String; amountInCoins: Cardinal; deviceId: Cardinal;
  metadata: array of TMetadata; cashierId: String);
var
  error: TCJetBeepError;
begin
  error := jetbeep_easypay_make_payment_partials(handle,
    PAnsiChar(AnsiString(merchantTransactionId)),
    PAnsiChar(AnsiString(paymentToken)), amountInCoins, deviceId, @metadata[0],
    Length(metadata), CPaymentResultHandler, THandle(Self),
    PAnsiChar(AnsiString(cashierId)));

  case error of
    JETBEEP_NO_ERROR:
      Exit;
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure TEasyPayBackend.MakeRefund(easyPayTransactionId: LongInt;
  amountInCoins: Cardinal; deviceId: Cardinal);
var
  error: TCJetBeepError;
begin
  error := jetbeep_easypay_make_refund(handle, easyPayTransactionId,
    amountInCoins, deviceId, CRefundResultHandler, THandle(Self));

  case error of
    JETBEEP_NO_ERROR:
      Exit;
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

procedure TEasyPayBackend.MakeRefundPartials(easyPayTransactionId: LongInt;
  amountInCoins: Cardinal; deviceId: Cardinal);
var
  error: TCJetBeepError;
begin
  error := jetbeep_easypay_make_refund_partials(handle, easyPayTransactionId,
    amountInCoins, deviceId, CRefundResultHandler, THandle(Self));

  case error of
    JETBEEP_NO_ERROR:
      Exit;
  else
    raise EJetBeepIO.Create('System input-output exception');
  end;
end;

end.
