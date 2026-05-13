; GhostSurf VST3 Plugin — Inno Setup Script
; Compile with Inno Setup 6: https://jrsoftware.org/isinfo.php

#define MyAppName "GhostSurf"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "WipeOut Audio"
#define VST3File "GhostSurf.vst3"

[Setup]
AppId={{B2A4C9E1-7F3D-4E8A-9B1C-2D5F6E7A8B9C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppComments=Vintage Surf Rock VST3 Plugin
DefaultDirName={commonpf64}\VSTPlugins
DefaultGroupName={#MyAppPublisher}
OutputDir=Installer
OutputBaseFilename=GhostSurf_Setup_v{#MyAppVersion}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
WizardImageFile=compiler:WizModernImage.bmp
UninstallDisplayName={#MyAppName} VST3 Plugin
UninstallDisplayIcon={app}\{#VST3File}\Contents\x86_64-win\GhostSurf.dll
MinVersion=10.0
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french";  MessagesFile: "compiler:Languages\French.isl"

[Files]
; Main VST3 bundle (folder)
Source: "build\GhostSurf_artefacts\Release\VST3\GhostSurf.vst3\*"; \
        DestDir: "{code:GetVST3Dir}\GhostSurf.vst3"; \
        Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName} README"; Filename: "{app}\README.txt"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

[Code]
// Let user choose VST3 folder (default = Common Files\VST3)
var
  VST3DirPage: TInputDirWizardPage;

procedure InitializeWizard;
begin
  VST3DirPage := CreateInputDirPage(wpSelectDir,
    'VST3 Plugin Folder',
    'Where should GhostSurf.vst3 be installed?',
    'Select the folder where your DAW loads VST3 plugins.' + #13#10 +
    'Default is recognized by FL Studio, Ableton, Reaper, etc.',
    False, '');
  VST3DirPage.Add('');
  VST3DirPage.Values[0] := ExpandConstant('{commonpf64}\Common Files\VST3');
end;

function GetVST3Dir(Param: String): String;
begin
  Result := VST3DirPage.Values[0];
end;

[Run]
Filename: "{cmd}"; Parameters: "/c echo GhostSurf installed successfully!"; \
          Flags: runhidden

[Messages]
english.WelcomeLabel1=Welcome to [name] Setup
english.WelcomeLabel2=This will install GhostSurf v{#MyAppVersion} VST3 plugin on your computer.%n%nVintage Spring Reverb, Tremolo, Tube Saturation, LoFi and EQ — all in one plugin.%n%nClick Next to continue.
