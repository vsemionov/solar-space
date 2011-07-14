#define MyAppName "Solar Space"
#define MyAppVersion "1.2.0"
#define MyAppPublisher "Victor Semionov"
#define MyAppPublisherShort "vsemionov"
#define MyPublisherURL "http://www.vsemionov.org/"
#define MyAppURL "http://www.vsemionov.org/solar-space/"
#define MyAppExeName "SolSpace.scr"

[Setup]
AppId={{DEA29385-E983-4A44-81AB-F9CA69961459}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=..\LICENSE.txt
InfoAfterFile=..\AUTHORS.txt
OutputDir=..\build
OutputBaseFilename={#MyAppName} {#MyAppVersion}
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "setsaver"; Description: "&Set as the current screen saver"

[Files]
Source: "..\Release\{#MyAppExeName}"; DestDir: "{win}"; Flags: ignoreversion
Source: "..\freetype6.dll"; DestDir: "{sys}"; Flags: sharedfile
Source: "..\zlib1.dll"; DestDir: "{sys}"; Flags: sharedfile
Source: "..\Solar Space.d2"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Solar System.d2"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.txt"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\AUTHORS.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\CHANGES.txt"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
Root: HKLM; Subkey: "Software\{#MyAppPublisherShort}\{#MyAppName}"; ValueType: string; ValueName: "Data Directory"; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Control Panel\Desktop"; ValueType: string; ValueName: "SCRNSAVE.EXE"; ValueData: "{code:GetShortName|{win}\{#MyAppExeName}}"; Tasks: setsaver

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{win}\{#MyAppExeName}"; Parameters: "-s"
Name: "{group}\{#MyAppName} Options"; Filename: "{win}\{#MyAppExeName}"; Parameters: "-c 0"; IconIndex: 1
Name: "{commondesktop}\{#MyAppName}"; Filename: "{win}\{#MyAppExeName}"; Parameters: "-s"; Tasks: desktopicon

[Run]
Filename: "{win}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Parameters: "-s"; Flags: nowait postinstall skipifsilent

