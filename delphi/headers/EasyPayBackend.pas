unit EasyPayBackend;

interface
  uses classes, System.SysUtils, EasyPayBackendImport, JetBeepTypes;

  type
    TEasyPayPaymentResult = record
      errorString: String;
      easyPayTransactionId: Integer;
      easyPayPaymentRequestUid: String;
    end;

    TEasyPayRefundResult = record
      errorString: String;
    end;

    TEasyPayPaymentResultHandler = reference to procedure(result: TEasyPayPaymentResult);
    TEasyPayRefundResultHandler = reference to procedure(result: TEasyPayRefundResult);

    TEasyPayBackend = class(TObject)
      private
        handle: TEasyPayBackendHandle;
      public
        paymentResultHandler: TEasyPayPaymentResultHandler;
        refundResultHandler: TEasyPayRefundResultHandler;

        constructor Create(environment: TEasyPayEnvironment; merchantKey: String);
        destructor Destroy; override;

        procedure MakePayment(merchantTransactionId: String; paymentToken: String; amountInCoins: Cardinal; deviceId: Cardinal; cashierId: String);
        procedure MakePaymentPartials(merchantTransactionId: String; paymentToken: String; amountInCoins: Cardinal; deviceId: Cardinal; metadata: array of TMetadata; cashierId: String);
        procedure MakeRefund(easypayTransactionId: LongInt; amountInCoins: Cardinal; deviceId: Cardinal);
        procedure MakeRefundPartials(easypayTransactionId: LongInt; amountInCoins: Cardinal; deviceId: Cardinal);
    end;

    procedure CPaymentResultHandler(result: TCEasyPayPaymentResult; data: THandle); cdecl;
    procedure CRefundResultHandler(result: TCEasyPayRefundResult; data: THandle); cdecl;

implementation
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

      ConvertedResult.errorString := String(result.errorString);
      ConvertedResult.easyPayTransactionId := result.easyPayTransactionId;
      ConvertedResult.easyPayPaymentRequestUid := String(result.easyPayPaymentRequestUid);
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

      ConvertedResult.errorString := String(result.errorString);
      EasyPayBackend.refundResultHandler(ConvertedResult);
    end;

    constructor TEasyPayBackend.Create(environment: TEasyPayEnvironment; merchantKey: String);
    begin
      inherited Create;
      handle:= jetbeep_easypay_new(environment, PAnsiChar(AnsiString(merchantKey)));
    end;

    destructor TEasyPayBackend.Destroy;
    begin
       jetbeep_easypay_free(handle);
       inherited;
    end;

    procedure TEasyPayBackend.MakePayment(merchantTransactionId: String; paymentToken: String; amountInCoins: Cardinal; deviceId: Cardinal; cashierId: String);
    var
      error: TCJetBeepError;
    begin
      Writeln(THandle(Self));
      error:= jetbeep_easypay_make_payment(handle, PAnsiChar(AnsiString(merchantTransactionId)), PAnsiChar(AnsiString(paymentToken)), amountInCoins, deviceId, CPaymentResultHandler, THandle(Self), PAnsiChar(AnsiString(cashierId)));

      case error of
      JETBEEP_NO_ERROR: Exit;
      else raise EJetBeepIO.Create('System input-output exception');
      end;
    end;

    procedure TEasyPayBackend.MakePaymentPartials(merchantTransactionId: String; paymentToken: String; amountInCoins: Cardinal; deviceId: Cardinal; metadata: array of TMetadata; cashierId: String);
    var
      error: TCJetBeepError;
    begin
      error:= jetbeep_easypay_make_payment_partials(handle, PAnsiChar(AnsiString(merchantTransactionId)), PAnsiChar(AnsiString(paymentToken)), amountInCoins, deviceId, @metadata[0], Length(metadata), CPaymentResultHandler, THandle(Self), PAnsiChar(AnsiString(cashierId)));

      case error of
      JETBEEP_NO_ERROR: Exit;
      else raise EJetBeepIO.Create('System input-output exception');
      end;
    end;

    procedure TEasyPayBackend.MakeRefund(easypayTransactionId: LongInt; amountInCoins: Cardinal; deviceId: Cardinal);
    var
      error: TCJetBeepError;
    begin
      error:= jetbeep_easypay_make_refund(handle, easypayTransactionId, amountInCoins, deviceId, CRefundResultHandler, THandle(Self));

      case error of
      JETBEEP_NO_ERROR: Exit;
      else raise EJetBeepIO.Create('System input-output exception');
      end;
    end;

    procedure TEasyPayBackend.MakeRefundPartials(easypayTransactionId: LongInt; amountInCoins: Cardinal; deviceId: Cardinal);
    var
      error: TCJetBeepError;
    begin
      error:= jetbeep_easypay_make_refund_partials(handle, easypayTransactionId, amountInCoins, deviceId, CRefundResultHandler, THandle(Self));

      case error of
      JETBEEP_NO_ERROR: Exit;
      else raise EJetBeepIO.Create('System input-output exception');
      end;
    end;
end.
