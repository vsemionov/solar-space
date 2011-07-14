#define MyAppName "Solar Space"
#define MyAppVersion "1.2.0"
#define MyAppPublisher "Victor Semionov"
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

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Flags: nowait postinstall skipifsilent

