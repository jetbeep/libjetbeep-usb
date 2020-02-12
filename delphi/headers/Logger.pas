{ Libjetbeep internal logger system }
unit Logger;

interface

uses classes, System.SysUtils, LoggerImport, JetBeepTypes, VersionChecker;

type

  { Libjetbeep logger controller }
  TLogger = class(TObject)
  private
    class function GetLogLevel: TJetBeepLogLevel; static;
    class procedure SetLogLevel(const Value: TJetBeepLogLevel); static;
    class function GetCoutEnabled: Boolean; static;
    class procedure SetCoutEnabled(const Value: Boolean); static;
    class function GetCerrEnabled: Boolean; static;
    class procedure SetCerrEnabled(const Value: Boolean); static;
  public
    { Sets\Gets logging level of the library. Default is: JETBEEP_LOGGER_SILENT
      @raises EJetBeepInvalidVersion Raised if jetbeep-*.dll version is not equal to *.pas version. IMPORTANT: Do not try to catch this exception as it means you're doing something wrong }
    class property level: TJetBeepLogLevel Read GetLogLevel Write SetLogLevel;

    { Sets\ Gets if logs should be redirected to stdout
      @raises EJetBeepInvalidVersion Raised if jetbeep-*.dll version is not equal to *.pas version. IMPORTANT: Do not try to catch this exception as it means you're doing something wrong }
    class property coutEnabled: Boolean Read GetCoutEnabled
      Write SetCoutEnabled;

    { Sets\ Gets if logs should be redirected to stderr
      @raises EJetBeepInvalidVersion Raised if jetbeep-*.dll version is not equal to *.pas version. IMPORTANT: Do not try to catch this exception as it means you're doing something wrong }
    class property cerrEnabled: Boolean Read GetCerrEnabled
      Write SetCerrEnabled;
  end;
  { TLogger }

implementation

class function TLogger.GetCerrEnabled: Boolean;
begin
  TVersionChecker.check;
  Result := jetbeep_logger_is_cerr_enabled;
end;

class function TLogger.GetCoutEnabled: Boolean;
begin
  TVersionChecker.check;
  Result := jetbeep_logger_is_cout_enabled;
end;

class function TLogger.GetLogLevel: TJetBeepLogLevel;
begin
  TVersionChecker.check;
  Result := jetbeep_logger_get_level();
end;

class procedure TLogger.SetCerrEnabled(const Value: Boolean);
begin
  TVersionChecker.check;
  jetbeep_logger_set_cerr_enabled(Value);
end;

class procedure TLogger.SetCoutEnabled(const Value: Boolean);
begin
  TVersionChecker.check;
  jetbeep_logger_set_cout_enabled(Value);
end;

class procedure TLogger.SetLogLevel(const Value: TJetBeepLogLevel);
begin
  TVersionChecker.check;
  jetbeep_logger_set_level(Value);
end;

end.
