;; ============================================================
 ;
 ; This file is a part of digiKam project
 ; http://www.digikam.org
 ;
 ; Date        : 2010-11-08
 ; Description : Null Soft windows installer based for digiKam
 ;
 ; Copyright (C) 2010 by Julien Narboux <julien at narboux dot fr>
 ; Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 ;
 ; Script arguments:
 ; VERSION  : the digiKam version string.
 ; KDE4PATH : the path where whole KDE4 + digiKam & co is installed.
 ;
 ; Example: makensis.exe -DVERSION=1.6.0 -DKDE4PATH=D:\kde4 digikam.nsi
 ;
 ; NSIS script reference can be found at this url:
 ; http://nsis.sourceforge.net/Docs/Chapter4.html
 ;
 ; This program is free software; you can redistribute it
 ; and/or modify it under the terms of the GNU General
 ; Public License as published by the Free Software Foundation;
 ; either version 2, or (at your option)
 ; any later version.
 ;
 ; This program is distributed in the hope that it will be useful,
 ; but WITHOUT ANY WARRANTY; without even the implied warranty of
 ; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ; GNU General Public License for more details.
 ;
 ; ============================================================ ;;

;-------------------------------------------------------------------------------
; Compression rules optimizations
; We will use LZMA compression as 7Zip, with a dictionary size of 96Mb (to reproduce 7Zip Ultra compression mode)

SetCompress force
SetCompressor /SOLID lzma
SetDatablockOptimize on
SetCompressorDictSize 96

;-------------------------------------------------------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !define MY_PRODUCT "digiKam"
  !define PRODUCT_HOMEPAGE "http://www.digikam.org"
  !define SUPPORT_HOMEPAGE "http://www.digikam.org/support"
  !define ABOUT_HOMEPAGE "http://www.digikam.org/about"
  !define OUTFILE "${MY_PRODUCT}-installer-${VERSION}-win32.exe"
  !define MSVCRuntimePath "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT"

;-------------------------------------------------------------------------------
;General

  ;Name and file
  Name "${MY_PRODUCT}"
  Icon "digikam-installer.ico"
  UninstallIcon "digikam-uninstaller.ico"
  OutFile "${OUTFILE}"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${MY_PRODUCT}"

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\${MY_PRODUCT}" ""

  ;Request application privileges for Windows Vista
  ;Requires UAC plugin:
  ;http://nsis.sourceforge.net/UAC_plug-in
  ;Copy UAC.dll to NSIS 'Plugins', UAC.nsh to '', and the two .nsi to 'Examples'
  !include "UAC.nsh"
  RequestExecutionLevel admin

  Function .onInit
    #TODO: call UserInfo plugin to make sure user is admin
  FunctionEnd

;-------------------------------------------------------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "digikam_header.bmp" 
  !define MUI_WELCOMEFINISHPAGE_BITMAP "digikam_welcome.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "digikam_welcome.bmp"
  !define MUI_ABORTWARNING
  !define MUI_ICON "digikam-installer.ico"
  !define MUI_UNICON "digikam-uninstaller.ico"
  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\RELEASENOTES.txt"

  ;Variable for the folder of the start menu
  Var StartMenuFolder

;-------------------------------------------------------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${MY_PRODUCT}"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;-------------------------------------------------------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SpanishInternational"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Korean"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
  !insertmacro MUI_LANGUAGE "NorwegianNynorsk"
  !insertmacro MUI_LANGUAGE "Finnish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Croatian"
  !insertmacro MUI_LANGUAGE "Bulgarian"
  !insertmacro MUI_LANGUAGE "Hungarian"
  !insertmacro MUI_LANGUAGE "Thai"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Latvian"
  !insertmacro MUI_LANGUAGE "Macedonian"
  !insertmacro MUI_LANGUAGE "Estonian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Lithuanian"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Serbian"
  !insertmacro MUI_LANGUAGE "SerbianLatin"
  !insertmacro MUI_LANGUAGE "Arabic"
  !insertmacro MUI_LANGUAGE "Farsi"
  !insertmacro MUI_LANGUAGE "Hebrew"
  !insertmacro MUI_LANGUAGE "Indonesian"
  !insertmacro MUI_LANGUAGE "Mongolian"
  !insertmacro MUI_LANGUAGE "Luxembourgish"
  !insertmacro MUI_LANGUAGE "Albanian"
  !insertmacro MUI_LANGUAGE "Breton"
  !insertmacro MUI_LANGUAGE "Belarusian"
  !insertmacro MUI_LANGUAGE "Icelandic"
  !insertmacro MUI_LANGUAGE "Malay"
  !insertmacro MUI_LANGUAGE "Bosnian"
  !insertmacro MUI_LANGUAGE "Kurdish"
  !insertmacro MUI_LANGUAGE "Irish"
  !insertmacro MUI_LANGUAGE "Uzbek"
  !insertmacro MUI_LANGUAGE "Galician"
  !insertmacro MUI_LANGUAGE "Afrikaans"
  !insertmacro MUI_LANGUAGE "Catalan"
  !insertmacro MUI_LANGUAGE "Esperanto"

;-------------------------------------------------------------------------------
;Installer Sections

Section "digiKam" SecDigiKam

  ;First we kill the running process.
  nsExec::Exec 'taskkill /f /im dbus-daemon.exe'
	Pop $0 # return value/error/timeout
  nsExec::Exec 'taskkill /f /im kded4.exe'
	Pop $0 # return value/error/timeout
  nsExec::Exec 'taskkill /f /im kioslave.exe'
	Pop $0 # return value/error/timeout
  nsExec::Exec 'taskkill /f /im klauncher.exe'
	Pop $0 # return value/error/timeout
  nsExec::Exec 'taskkill /f /im digikam.exe'
	Pop $0 # return value/error/timeout
  
  SetOutPath "$INSTDIR"

  File "RELEASENOTES.txt"
  File "digikam-uninstaller.ico"

  ;Copy only required directories
  ;The SetOutPath is required because otherwise NSIS will assume all files are
  ;  in the same folder even though they are source from different folders
  ;The \*.* is required because without it, NSIS would add every folder with
  ;  the name 'bin' in all subdirectories of $INSTDIR
  SetOutPath "$INSTDIR\bin"
  File "${MSVCRuntimePath}\msvcp100.dll"
  File "${MSVCRuntimePath}\msvcr100.dll"
  File /r "${KDE4PATH}\bin\*.*"
  SetOutPath "$INSTDIR\certs"
  File /r "${KDE4PATH}\certs\*.*"
  ;SetOutPath "$INSTDIR\data"
  ;File /r "${KDE4PATH}\data\*.*"
  ;SetOutPath "$INSTDIR\database"
  ;File /r "${KDE4PATH}\database\*.*"
  ;SetOutPath "$INSTDIR\doc"
  ;File /r "${KDE4PATH}\doc\*.*"
  SetOutPath "$INSTDIR\etc"
  File /r /x kdesettings.bat /x portage "${KDE4PATH}\etc\*.*"
  SetOutPath "$INSTDIR\hosting"
  File /r "${KDE4PATH}\hosting\*.*"
  SetOutPath "$INSTDIR\imports"
  File /r "${KDE4PATH}\imports\*.*"
  SetOutPath "$INSTDIR\include"
  File /r "${KDE4PATH}\include\*.*"
  SetOutPath "$INSTDIR\lib"
  File /r "${KDE4PATH}\lib\*.*"
  ;SetOutPath "$INSTDIR\manifest"
  ;File /r "${KDE4PATH}\manifest\*.*"
  SetOutPath "$INSTDIR\phrasebooks"
  File /r "${KDE4PATH}\phrasebooks\*.*"
  SetOutPath "$INSTDIR\plugins"
  File /r "${KDE4PATH}\plugins\*.*"
  ;SetOutPath "$INSTDIR\scripts"
  ;File /r "${KDE4PATH}\scripts\*.*"
  SetOutPath "$INSTDIR\share"
  File /r "${KDE4PATH}\share\*.*"
  SetOutPath "$INSTDIR\translations"
  File /r "${KDE4PATH}\translations\*.*"
  ;SetOutPath "$INSTDIR\vad"
  ;File /r "${KDE4PATH}\vad\*.*"
  ;SetOutPath "$INSTDIR\vsp"
  ;File /r "${KDE4PATH}\vsp\*.*"
  SetOutPath "$INSTDIR\xdg"
  File /r "${KDE4PATH}\xdg\*.*"

  ;Store installation folder
  WriteRegStr HKLM "Software\${MY_PRODUCT}" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Register uninstaller in windows registery with only the option to uninstall (no repair nor modify)
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "Comments" "${MY_PRODUCT} ${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "DisplayIcon" '"$INSTDIR\bin\digikam.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "DisplayName" "${MY_PRODUCT} ${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "HelpLink" "${SUPPORT_HOMEPAGE}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "Publisher" "The digiKam team"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "Readme" '"$INSTDIR\RELEASENOTES.txt"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "URLInfoAbout" "${ABOUT_HOMEPAGE}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "URLUpdateInfo" "${PRODUCT_HOMEPAGE}"
  ;Calculate the install size so that it can be shown in the uninstall interface in Windows
  ;see http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs
  ;this isn't the most accurate method but it is very fast and is accurate enough for an estimate
  push $0
  SectionGetSize SecDigiKam $0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "EstimatedSize" "$0"
  pop $0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "NoRepair" "1"
  ;WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "VersionMajor" "2"
  ;WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "VersionMinor" "2"

  ;Add start menu items
  SetShellVarContext all
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    SetOutPath "$INSTDIR"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    SetOutPath "$INSTDIR\bin"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${MY_PRODUCT}.lnk" "$INSTDIR\bin\digikam.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Showfoto.lnk" "$INSTDIR\bin\showfoto.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\DNGConverter.lnk" "$INSTDIR\bin\dngconverter.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\ExpoBlending.lnk" "$INSTDIR\bin\expoblending.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\ScanGui.lnk" "$INSTDIR\bin\scangui.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\SystemSettings.lnk" "$INSTDIR\bin\systemsettings.exe"
    WriteINIStr "$SMPROGRAMS\$StartMenuFolder\The ${MY_PRODUCT} HomePage.url" "InternetShortcut" "URL" "${PRODUCT_HOMEPAGE}"

  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;-------------------------------------------------------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete /REBOOTOK "$INSTDIR\Uninstall.exe"
  Delete /REBOOTOK "$INSTDIR\RELEASENOTES.txt"
  Delete /REBOOTOK "$INSTDIR\digikam-uninstaller.ico"

  RMDir /r /REBOOTOK "$INSTDIR\bin"
  RMDir /r /REBOOTOK "$INSTDIR\certs"
  ;RMDir /r /REBOOTOK "$INSTDIR\data"
  ;RMDir /r /REBOOTOK "$INSTDIR\database"
  ;RMDir /r /REBOOTOK "$INSTDIR\doc"
  RMDir /r /REBOOTOK "$INSTDIR\etc"
  RMDir /r /REBOOTOK "$INSTDIR\hosting"
  RMDir /r /REBOOTOK "$INSTDIR\imports"
  RMDir /r /REBOOTOK "$INSTDIR\include"
  RMDir /r /REBOOTOK "$INSTDIR\lib"
  ;RMDir /r /REBOOTOK "$INSTDIR\manifest"
  RMDir /r /REBOOTOK "$INSTDIR\phrasebooks"
  RMDir /r /REBOOTOK "$INSTDIR\plugins"
  ;RMDir /r /REBOOTOK "$INSTDIR\scripts"
  RMDir /r /REBOOTOK "$INSTDIR\share"
  RMDir /r /REBOOTOK "$INSTDIR\translations"
  ;RMDir /r /REBOOTOK "$INSTDIR\vad"
  ;RMDir /r /REBOOTOK "$INSTDIR\vsp"
  RMDir /r /REBOOTOK "$INSTDIR\xdg"

  ;Do not do a recursive removal of $INSTDIR because user may have accidentally installed into system critical directory!
  RMDir /REBOOTOK "$INSTDIR"

  ;Remove start menu items
  SetShellVarContext all
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${MY_PRODUCT}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Showfoto.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\DNGConverter.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\ExpoBlending.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\ScanGui.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\SystemSettings.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\The ${MY_PRODUCT} HomePage.url"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  ;Remove registry entries
  DeleteRegKey /ifempty HKLM "Software\${MY_PRODUCT}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}"

SectionEnd
