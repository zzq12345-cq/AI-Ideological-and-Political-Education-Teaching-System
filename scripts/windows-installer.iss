#define MyAppName "AI思政智慧课堂"
#ifndef MyAppVersion
  #define MyAppVersion "2.1.5"
#endif
#define MyAppPublisher "智慧教育科技有限公司"
#define MyAppExeName "AILoginSystem.exe"
#ifndef MyOutputBaseFilename
  #define MyOutputBaseFilename "AI思政智慧课堂-Setup-Windows-x64-2.1.5"
#endif
#ifndef BuildRoot
  #define BuildRoot "..\build"
#endif
#ifndef OutputDir
  #define OutputDir BuildRoot
#endif

[Setup]
AppId={{8B2E91A3-563E-4F4B-8D35-6E4D5E8A71F2}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}
OutputDir={#OutputDir}
OutputBaseFilename={#MyOutputBaseFilename}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
SetupIconFile=..\resources\AppIcon_transparent.ico

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "创建桌面快捷方式"; GroupDescription: "附加任务:"; Flags: unchecked

[Files]
Source: "{#BuildRoot}\deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\卸载 {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "启动 {#MyAppName}"; Flags: nowait postinstall skipifsilent
