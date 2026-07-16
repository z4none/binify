#define AppName "binify"
#define AppVersion "0.1.0"

#ifndef AppArch
  #define AppArch "x64"
#endif

#if AppArch == "x64"
  #define BuildPreset "windows-x64-release"
  #define ArchitectureAllowed "x64compatible"
  #define InstallIn64BitMode "x64compatible"
#elif AppArch == "arm64"
  #define BuildPreset "windows-arm64-release"
  #define ArchitectureAllowed "arm64"
  #define InstallIn64BitMode "arm64"
#elif AppArch == "x86"
  #define BuildPreset "windows-x86-release"
  #define ArchitectureAllowed "x86compatible"
#else
  #error Unsupported AppArch. Use /DAppArch=x64, /DAppArch=arm64, or /DAppArch=x86.
#endif

#define BuildOutput "..\build\" + BuildPreset + "\Release"
#define AppExe BuildOutput + "\binify.exe"

#if !FileExists(AppExe)
  #error Build output not found. Run: cmake --build --preset {#BuildPreset}
#endif

[Setup]
AppId={{A4C1E33A-C6F4-48F2-80BD-22B65DA37D60}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=binify contributors
DefaultDirName={localappdata}\Programs\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
LicenseFile=license.txt
OutputDir=..\out\installer
OutputBaseFilename={#AppName}-{#AppVersion}-windows-{#AppArch}
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=lowest
ArchitecturesAllowed={#ArchitectureAllowed}
#if AppArch != "x86"
ArchitecturesInstallIn64BitMode={#InstallIn64BitMode}
#endif
UninstallDisplayIcon={app}\binify.exe
SetupLogging=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "license.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\third_party\winlamb\LICENSE.txt"; DestDir: "{app}\licenses"; DestName: "winlamb-LICENSE.txt"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\binify.exe"

[Run]
Filename: "{app}\binify.exe"; Description: "Launch {#AppName}"; Flags: postinstall nowait skipifsilent unchecked

[UninstallRun]
Filename: "{app}\binify.exe"; Parameters: "--uninstall-cleanup"; RunOnceId: "binify-uninstall-cleanup"; Flags: skipifdoesntexist
