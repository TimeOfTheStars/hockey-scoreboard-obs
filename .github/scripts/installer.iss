; Установщик плагина «Табло — пульт трансляции» для Windows (Inno Setup 6).
; Собирается в CI из Package-Windows.ps1; версия и пути приходят ключами /D.

#ifndef Version
  #define Version "0.0.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\..\release\RelWithDebInfo\hockey-scoreboard-obs"
#endif
#ifndef OutputDir
  #define OutputDir "..\..\release"
#endif
#ifndef OutputName
  #define OutputName "hockey-scoreboard-obs-windows-x64-Installer"
#endif

#define AppName "Табло — пульт трансляции"

[Setup]
AppId={{127AD357-D091-4596-BD88-48D738BEE90B}
AppName={#AppName}
AppVersion={#Version}
AppVerName={#AppName} {#Version}
AppPublisher=TimeOfTheStars
AppPublisherURL=https://github.com/TimeOfTheStars/hockey-scoreboard-obs
AppSupportURL=https://github.com/TimeOfTheStars/hockey-scoreboard-obs/issues
AppUpdatesURL=https://github.com/TimeOfTheStars/hockey-scoreboard-obs/releases/latest
; Папка плагинов OBS 30+ для всех пользователей: OBS находит плагин сам, а
; программа и её обновления не трогаются.
DefaultDirName={commonappdata}\obs-studio\plugins\hockey-scoreboard-obs
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#OutputDir}
OutputBaseFilename={#OutputName}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
; При обновлении библиотеку плагина держит запущенный OBS — предлагаем его закрыть.
CloseApplications=yes
RestartApplications=no
UninstallDisplayName={#AppName}
VersionInfoVersion={#Version}
VersionInfoCompany=TimeOfTheStars
VersionInfoDescription=Плагин OBS: пульт трансляции хоккейного табло

[Languages]
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "en"; MessagesFile: "compiler:Default.isl"

[Files]
; Отладочные символы в установщик не берём: они нужны разработчику, а не OBS.
Source: "{#SourceDir}\*"; DestDir: "{app}"; Excludes: "*.pdb"; Flags: ignoreversion recursesubdirs createallsubdirs

[UninstallDelete]
Type: dirifempty; Name: "{app}"

[Messages]
ru.FinishedLabel=Плагин установлен.%n%nЗапустите OBS: в главном окне появится меню «Табло» и док «Табло». Подключите OBS кодом с экрана подготовки: «Пульты и OBS» → «Подключить OBS».
en.FinishedLabel=The plugin is installed.%n%nStart OBS: the “Scoreboard” menu and dock appear in the main window. Connect OBS with the code from the setup screen: “Consoles and OBS” → “Connect OBS”.
