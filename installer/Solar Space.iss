
; Copyright (C) 2003-2011 Victor Semionov
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;  * Redistributions of source code must retain the above copyright notice, this
;    list of conditions and the following disclaimer.
;  * Redistributions in binary form must reproduce the above copyright notice,
;    this list of conditions and the following disclaimer in the documentation
;    and/or other materials provided with the distribution.
;  * Neither the name of the copyright holder nor the names of the contributors
;    may be used to endorse or promote products derived from this software
;    without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#define MyAppName "Solar Space"
; the version is also specified below for VersionInfoVersion
#define MyAppVersion "1.2"
#define MyAppPublisher "Victor Semionov"
#define MyAppPublisherShort "vsemionov"
#define MyPublisherURL "http://www.vsemionov.org/"
#define MyAppURL "http://www.vsemionov.org/solar-space/"
#define MyAppExeName "Solar Space.exe"
#define MyAppScrName "SolSpace.scr"
#define MyCopyrightHolder MyAppPublisher
#define MyCopyrightPeriod "2003-2011"


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
OutputDir=..\dist
OutputBaseFilename={#MyAppName} Setup {#MyAppVersion}
Compression=lzma
SolidCompression=yes

; keep the file version in full format
VersionInfoVersion=1.2.0.0
VersionInfoCopyright=Copyright (C) {#MyCopyrightPeriod} {#MyCopyrightHolder}
SetupIconFile=..\src\res\Solar Space.ico
UninstallDisplayIcon={app}\{#MyAppExeName}


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "setsaver"; Description: "&Set {#MyAppName} as the current screen saver"; GroupDescription: "Miscellaneous:"

[Files]
Source: "..\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\{#MyAppExeName}"; DestDir: "{win}"; DestName: "{#MyAppScrName}"; Flags: ignoreversion
Source: "..\freetype6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Solar Space.d2"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Solar System.d2"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.rst"; DestDir: "{app}"; DestName: "README.txt"; Flags: ignoreversion
Source: "..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\AUTHORS.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\CHANGES.txt"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
Root: HKLM; Subkey: "Software\{#MyAppPublisherShort}\{#MyAppName}"; ValueType: string; ValueName: "Data Directory"; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Control Panel\Desktop"; ValueType: string; ValueName: "SCRNSAVE.EXE"; ValueData: "{code:GetShortName|{win}\{#MyAppScrName}}"; Tasks: setsaver

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Parameters: "-s"
Name: "{group}\Settings"; Filename: "{app}\{#MyAppExeName}"; Parameters: "-c 0"; IconIndex: 1
Name: "{group}\Readme"; Filename: "{app}\README.txt"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Parameters: "-s"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, "&", "&&")}}"; Parameters: "-s"; Flags: nowait postinstall skipifsilent unchecked
Filename: "{app}\README.txt"; Description: "View Readme"; Flags: shellexec skipifdoesntexist postinstall skipifsilent unchecked
