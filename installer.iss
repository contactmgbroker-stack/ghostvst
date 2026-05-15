[Setup]
AppName=GhostSurf
AppVersion=1.0.0
AppPublisher=WipeOut Audio - Martin Gullotte Broker
AppPublisherURL=https://mgbroker.ch/
AppSupportURL=https://mgbroker.ch/
AppUpdatesURL=https://mgbroker.ch/
DefaultDirName={commonpf64}\Common Files\VST3\GhostSurf.vst3
DefaultGroupName=GhostSurf
AllowNoIcons=yes
OutputBaseFilename=GhostSurf_Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=GhostSurf VST3
UninstallDisplayIcon={app}\Contents\x86_64-win\GhostSurf.vst3
PrivilegesRequired=admin
DisableProgramGroupPage=yes
CreateUninstallRegKey=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french";  MessagesFile: "compiler:Languages\French.isl"

[Files]
Source: "build\GhostSurf_artefacts\Release\VST3\GhostSurf.vst3\Contents\*"; \
  DestDir: "{app}\Contents"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Desinstaller GhostSurf"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\Contents\x86_64-win\GhostSurf.vst3"; \
  Description: "Ouvrir le dossier d'installation"; \
  Flags: shellexec skipifsilent postinstall runascurrentuser unchecked

[Messages]
english.WelcomeLabel1=Bienvenue dans l'installation de GhostSurf
english.WelcomeLabel2=GhostSurf est un plugin VST3 d'effets surf rock vintage.%n%nDeveloppe par WipeOut Audio - mgbroker.ch%n%nCliquez sur Suivant pour continuer.
french.WelcomeLabel1=Bienvenue dans l'installation de GhostSurf
french.WelcomeLabel2=GhostSurf est un plugin VST3 d'effets surf rock vintage.%n%nDeveloppe par WipeOut Audio - mgbroker.ch%n%nCliquez sur Suivant pour continuer.
