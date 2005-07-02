; Install program generation script for KevEdit in windows
; Requires Inno Setup

[Setup]
AppName=KevEdit
AppVerName=KevEdit 0.5.1
AppPublisher=Kev Vance and Ryan Phillips
AppPublisherURL=http://kevedit.sourceforge.net/
AppSupportURL=http://kevedit.sourceforge.net/contact.html
AppUpdatesURL=http://kevedit.sourceforge.net/download.html
DefaultDirName={pf}\KevEdit
DefaultGroupName=Games\ZZT
LicenseFile=COPYING.txt
InfoBeforeFile=README.txt
OutputDir=..
; Cosmetics
WindowVisible=no
WizardImageFile=install-big.bmp
WizardSmallImageFile=install-small.bmp

[Files]
Source: "AUTHORS.txt";  DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "ChangeLog.txt"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "COPYING.txt";  DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "kevedit.exe";  DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "kevedit.zml";  DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "README.txt";   DestDir: "{app}"; CopyMode: alwaysoverwrite; Flags: isreadme
Source: "README-SDL.txt"; Destdir: "{app}"; CopyMode: alwaysoverwrite
Source: "SDL.dll";      Destdir: "{app}"; CopyMode: alwaysoverwrite
;Stop releasing this file with KevEdit: it will be built automatically if needed
;Source: "..\default.zln";  Destdir: "{app}"; CopyMode: alwaysskipifsameorolder

[Icons]
Name: "{group}\KevEdit"; Filename: "{app}\kevedit.exe"; WorkingDir: "{userdocs}"
Name: "{group}\KevEdit Readme";  Filename: "{app}\README.txt"
Name: "{group}\Uninstall KevEdit"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\kevedit.exe"; Description: "Launch kevedit"; WorkingDir: "{userdocs}"; Flags: nowait postinstall skipifsilent

[Registry]
; Associate KevEdit with ZZT worlds
Root: HKCR; Subkey: ".zzt"; ValueType: string; ValueData: "zztfile"
Root: HKCR; Subkey: ".sav"; ValueType: string; ValueData: "zztsave"; Flags: createvalueifdoesntexist

Root: HKCR; Subkey: "zztfile\Shell\KevEdit"; ValueType: string; ValueData: "KevEdit"; Flags: uninsdeletekey
Root: HKCR; Subkey: "zztfile\Shell\KevEdit\Command"; ValueType: string; ValueData: "{app}\kevedit.exe ""%1"""; Flags: uninsdeletekey

Root: HKCR; Subkey: "zztsave\Shell"; ValueType: string; ValueData: "KevEdit"; Flags: uninsdeletekey
Root: HKCR; Subkey: "zztsave\Shell\KevEdit"; ValueType: string; ValueData: "KevEdit"; Flags: uninsdeletekey
Root: HKCR; Subkey: "zztsave\Shell\KevEdit\Command"; ValueType: string; ValueData: "{app}\kevedit.exe ""%1"""; Flags: uninsdeletekey

