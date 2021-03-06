[Setup]
AppName=xchat-bttrw
AppVerName=xchat-bttrw r. 537
OutputBaseFilename=xchat-bttrw-537
AppPublisher=NOMI team
AppPublisherURL=http://www.nomi.cz/
AppSupportURL=http://nomi.cz/projects.shtml?id=xchat-bttrw
AppUpdatesURL=http://nomi.cz/projects.shtml?id=xchat-bttrw
DefaultDirName={pf}\xchat-bttrw
DefaultGroupName=xchat-bttrw
AllowNoIcons=yes
LicenseFile=data\COPYING.txt
OutputDir=output
Compression=lzma
SolidCompression=yes
MinVersion=4.1,4.0
PrivilegesRequired=admin
WizardImageFile=WizImage.bmp
WizardSmallImageFile=WizImageSmall.bmp

[Languages]
Name: "eng"; MessagesFile: "compiler:Default.isl"
Name: "cze"; MessagesFile: "compiler:Languages\Czech-5-5.1.0.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: ;
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "data\gate.exe"; DestName: "gate.exe"; DestDir: "{app}"; Flags: ignoreversion; BeforeInstall: KillAll('gate.exe');
Source: "data\COPYING.txt";    DestDir: "{app}"; Flags: ignoreversion
Source: "data\COPYRIGHT";  DestDir: "{app}"; Flags: ignoreversion
Source: "data\config.exe";  DestDir: "{app}"; Flags: ignoreversion
Source: "data\run.bat";  DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist
Source: "data\killall.exe";  DestDir: "{app}"; Flags: ignoreversion
Source: "data\xchat\*";  DestDir: "{app}\xchat"; Flags: ignoreversion recursesubdirs; BeforeInstall: KillAll('xchat.exe');
Source: "data\ico\all.ico";  DestName: "gate.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "data\xchat-appdata\*";  DestDir: "{userappdata}\X-Chat 2"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "data\libiconv-2.dll"; DestDir: "{sys}"; BeforeInstall: KillAll('gate.exe');
Source: "data\mingwm10.dll"; DestDir: "{sys}"; BeforeInstall: KillAll('gate.exe');

;Knihovna Visual C++
Source: "data\msvcr71.dll"; DestDir: "{sys}"; Flags: restartreplace uninsneveruninstall sharedfile

[INI]
Filename: "{app}\gate.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.nomi.cz/projects.shtml?id=xchat-bttrw"
Filename: "{app}\gate-help.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://wiki.nomi.cz/xchat-bttrw:start"
Filename: "{app}\xchat.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.xchat.cz/"

[Icons]
Name: "{group}\xchat-bttrw"; Filename: "{app}\run.bat"; WorkingDir: "{app}"; IconFilename: "{app}\gate.ico"
Name: "{group}\Offici�ln� web xchat-bttrw"; Filename: "{app}\gate.url"
Name: "{group}\N�pov�da xchat-bttrw"; Filename: "{app}\gate-help.url"
Name: "{group}\Web www.xchat.cz"; Filename: "{app}\xchat.url"
Name: "{group}\{cm:UninstallProgram,xchat-bttrw}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\xchat-bttrw"; Filename: "{app}\run.bat"; Tasks: desktopicon; WorkingDir: "{app}"; IconFilename: "{app}\gate.ico"
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\xchat-bttrw"; Filename: "{app}\run.bat"; Tasks: quicklaunchicon;  WorkingDir: "{app}"; IconFilename: "{app}\gate.ico"

[Run]
Filename: "{app}\config.exe"; WorkingDir: "{userappdata}\X-Chat 2"; StatusMsg: "Nastavuji ..."; Flags: waituntilterminated
Filename: "{app}\run.bat"; Description: "{cm:LaunchProgram,xchat-bttrw}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\gate.url"
Type: files; Name: "{app}\gate-help.url"
Type: files; Name: "{app}\xchat.url"

[UninstallRun]
Filename: "{app}\killall.exe"; Parameters: "xchat.exe"; Flags: runhidden
Filename: "{app}\killall.exe"; Parameters: "gate.exe"; Flags: runhidden

[Code]
procedure KillAll(s: string);
var ResultCode: Integer;
begin
  Exec(ExpandConstant('{app}\killall.exe'), s, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;
