; Install program generation script for KevEdit in windows
; Requires Inno Setup

[Setup]
AppName=KevEdit
AppVerName=KevEdit 0.5.0
AppPublisher=Ryan Phillips
AppPublisherURL=mailto:bitman@users.sourceforge.net
AppSupportURL=mailto:bitman@users.sourceforge.net
AppUpdatesURL=mailto:bitman@users.sourceforge.net
DefaultDirName={pf}\Games\ZZTUWin
DefaultGroupName=Games\ZZT
AlwaysCreateUninstallIcon=yes
LicenseFile=..\Copying
InfoBeforeFile=..\windows.txt
OutputDir=..
; Cosmetics
WindowVisible=no
; TODO: make some artwork!
;WizardImageFile=..\images\install.bmp
;WizardSmallImageFile=..\images\instsmll.bmp

[Files]
Source: "..\kevedit.exe";  DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\kevedit.zml";  DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\windows.txt";  DestDir: "{app}"; CopyMode: alwaysoverwrite; Flags: isreadme
Source: "..\copying.txt";  DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\README";       Destdir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\TODO";         Destdir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\AUTHORS";      Destdir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\ChangeLog";      Destdir: "{app}"; CopyMode: alwaysoverwrite
; TODO: more? SDL.dll? Cygwin runtime?

[Icons]
Name: "{group}\KevEdit"; Filename: "{app}\kevedit.exe"
Name: "{group}\KevEdit Readme";  Filename: "{app}\windows.txt"

[Run]
Filename: "{app}\kevedit.exe"; Description: "Launch kevedit"; Flags: nowait postinstall skipifsilent

[Registry]
; Associate KevEdit with ZZT worlds
Root: HKCR; Subkey: ".zzt"; ValueType: string; ValueData: "zztfile"
Root: HKCR; Subkey: ".sav"; ValueType: string; ValueData: "zztsave"; Flags: createvalueifdoesntexist

Root: HKCR; Subkey: "zztfile\Shell\KevEdit"; ValueType: string; ValueData: "KevEdit"; Flags: uninsdeletekey
Root: HKCR; Subkey: "zztfile\Shell\KevEdit\Command"; ValueType: string; ValueData: "{app}\kevedit.exe %1"; Flags: uninsdeletekey

Root: HKCR; Subkey: "zztsave\Shell"; ValueType: string; ValueData: "KevEdit"; Flags: uninsdeletekey
Root: HKCR; Subkey: "zztsave\Shell\KevEdit"; ValueType: string; ValueData: "KevEdit"; Flags: uninsdeletekey
Root: HKCR; Subkey: "zztsave\Shell\KevEdit\Command"; ValueType: string; ValueData: "{app}\kevedit.exe %1"; Flags: uninsdeletekey

