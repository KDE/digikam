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
!define OUTFILE "${MY_PRODUCT}-installer-${VERSION}-win32.exe"

;-------------------------------------------------------------------------------
;General

  ;Name and file
  Name "digiKam"
  Icon "digikam-installer.ico"
  UninstallIcon "digikam-uninstaller.ico"
  OutFile "${OUTFILE}"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${MY_PRODUCT}"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${MY_PRODUCT}" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel highest

;-------------------------------------------------------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "digikam_header.bmp" 
  !define MUI_WELCOMEFINISHPAGE_BITMAP "digikam_welcome.bmp"
  !define MUI_ABORTWARNING
  !define MUI_ICON "digikam-installer.ico"
  !define MUI_UNICON "digikam-uninstaller.ico"
  !define MUI_FINISHPAGE_SHOWREADME "RELEASENOTES.txt"

;Variable for the folder of the start menu
Var StartMenuFolder

;-------------------------------------------------------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
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
  ;We use the following plugin:
  ;http://nsis.sourceforge.net/KillProc_plug-in
  ;To use this copy the dll in the Plugins directory of nsis.

  StrCpy $0 "dbus-daemon.exe"
  KillProc::KillProcesses
  StrCpy $0 "kded4.exe"
  KillProc::KillProcesses
  StrCpy $0 "kioslave.exe"
  KillProc::KillProcesses
  StrCpy $0 "klauncher.exe"
  KillProc::KillProcesses
  StrCpy $0 "digikam.exe"
  KillProc::KillProcesses

  SetOutPath "$INSTDIR"

  File "RELEASENOTES.txt"
  File "digikam-uninstaller.ico"

  ;Whole kde4 directory including digiKam & co
  File /r "${KDE4PATH}"

  ;Store installation folder
  WriteRegStr HKCU "Software\${MY_PRODUCT}" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Register uninstaller in windows registery with only the option to uninstall (no repair nor modify)
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "DisplayName" "${MY_PRODUCT} Version ${VERSION}"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "Publisher" "The digiKam team"
  WriteRegDWORD HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "NoModify" "1"
  WriteRegDWORD HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "NoRepair" "1"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "URLInfoAbout" "${PRODUCT_HOMEPAGE}"

 ;Add start menu
 !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${MY_PRODUCT}.lnk" "$INSTDIR\kde4\bin\digikam.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Showfoto.lnk" "$INSTDIR\kde4\bin\showfoto.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\DNGConverter.lnk" "$INSTDIR\kde4\bin\dngconverter.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\ExpoBlending.lnk" "$INSTDIR\kde4\bin\expoblending.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\ScanGui.lnk" "$INSTDIR\kde4\bin\scangui.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\SystemSettings.lnk" "$INSTDIR\kde4\bin\systemsettings.exe"
    WriteINIStr "$SMPROGRAMS\$StartMenuFolder\The ${MY_PRODUCT} HomePage.url" "InternetShortcut" "URL" "${PRODUCT_HOMEPAGE}"

 !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;-------------------------------------------------------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"
  Delete /REBOOTOK "$INSTDIR\RELEASENOTES.txt"
  Delete /REBOOTOK "$INSTDIR\digikam-uninstaller.ico"

  RMDir /r /REBOOTOK "$INSTDIR\kde4"

  RMDir "$INSTDIR"

  ;remove start menu items
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

  DeleteRegKey /ifempty HKCU "Software\${MY_PRODUCT}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}"

SectionEnd
