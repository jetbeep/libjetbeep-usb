{ Internal unit, you should NEVER use it }
unit LoggerImport;

interface

uses
  SysUtils, JetBeepTypes;

type
  TJetBeepLogLevel = (JETBEEP_LOGGER_VERBOSE = 0, JETBEEP_LOGGER_DEBUG,
    JETBEEP_LOGGER_INFO, JETBEEP_LOGGER_WARNING, JETBEEP_LOGGER_ERROR,
    JETBEEP_LOGGER_SILENT);

function jetbeep_logger_get_level: TJetBeepLogLevel; cdecl;
procedure jetbeep_logger_set_level(level: TJetBeepLogLevel); cdecl;
function jetbeep_logger_is_cout_enabled: Boolean; cdecl;
function jetbeep_logger_is_cerr_enabled: Boolean; cdecl;
procedure jetbeep_logger_set_cout_enabled(isEnabled: Boolean); cdecl;
procedure jetbeep_logger_set_cerr_enabled(isEnabled: Boolean); cdecl;

implementation

const
{$IFDEF WIN32}
  DLLName = 'jetbeep_i386.dll';
{$ELSEIF Defined(WIN64)}
  DLLName = 'jetbeep_x86_64.dll';
{$ELSE}
{$MESSAGE Fatal 'Platform not supported!'}
{$ENDIF}
function jetbeep_logger_get_level; external DLLName;
procedure jetbeep_logger_set_level; external DLLName;
function jetbeep_logger_is_cout_enabled; external DLLName;
function jetbeep_logger_is_cerr_enabled; external DLLName;
procedure jetbeep_logger_set_cout_enabled; external DLLName;
procedure jetbeep_logger_set_cerr_enabled; external DLLName;

end.
