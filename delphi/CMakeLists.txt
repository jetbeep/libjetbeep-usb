if (NOT WIN32)
    return()
endif()

file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/headers/VersionChecker.pas"
"unit VersionChecker;

interface

uses
  SysUtils, JetBeepTypes;

function jetbeep_get_version: PAnsiChar; cdecl;

type

  TVersionChecker = class(TObject)
  public
    class procedure check;
  end;

const
{$IFDEF WIN32}
  DLLName = 'jetbeep_i386.dll';
{$ELSEIF Defined(WIN64)}
  DLLName = 'jetbeep_x86_64.dll';
{$ELSE}
{$MESSAGE Fatal 'Platform not supported!'}
{$ENDIF}

const
  JetBeepLibraryVersion = '${PROJECT_VERSION}';

implementation

{ TLogger }

class procedure TVersionChecker.check;
var
  version: string;
begin
  version := string(AnsiString(jetbeep_get_version()));
  if version <> JetBeepLibraryVersion then
  begin
    raise EJetBeepInvalidVersion.Create
      ('Delphi and DLL library version mismatch! Please update *.pas and *.dll to the same version!');
  end;
end;

function jetbeep_get_version: PAnsiChar; external DLLName;

end."
)
