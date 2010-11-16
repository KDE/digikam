;; ============================================================
 ;
 ; This file is a part of digiKam project
 ; http://www.digikam.org
 ;
 ; Date        : 2010-11-08
 ; Description : Null Soft windows installer based for digiKam
 ;
 ; Copyright (C) 2010 by Julien Narboux <joern.ahrens@kdemail.net>
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
!define OUTFILE "${MY_PRODUCT}-installer-${VERSION}.exe"

;-------------------------------------------------------------------------------
;General

  ;Name and file
  Name "digiKam"
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

;Variable for the folder of the start menu
Var StartMenuFolder

;-------------------------------------------------------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${MY_PRODUCT}" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;-------------------------------------------------------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "Italian"

;-------------------------------------------------------------------------------
;Installer Sections

Section "digiKam" SecDigiKam

  SetOutPath "$INSTDIR"

  ;Whole kde4 directory including digiKam & co
  File /r "${KDE4PATH}"

  ; Extract the KDE4 path dir name. It must be the same in target install dir.
  ${Explode} $Size "\" ${KDE4PATH}
  ${For} $1 1 $Size
      Pop $2
  ${Next}
  DirName $2

  ;Store installation folder
  WriteRegStr HKCU "Software\${DirName}" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Register uninstaller in windows registery with only the option to uninstall (no repair nor modify)
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" \
          "DisplayName" "${MY_PRODUCT} Version ${VERSION}"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" \
          "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" \
          "DisplayVersion" "${VERSION}"
  WriteRegDWORD HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" \
          "NoModify" "1"
  WriteRegDWORD HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" \
          "NoRepair" "1"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" \
          "URLInfoAbout" "${PRODUCT_HOMEPAGE}"

 ;Add start menu
 !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${MY_PRODUCT}.lnk" "$INSTDIR\kde4\bin\digikam.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Showfoto.lnk" "$INSTDIR\kde4\bin\showfoto.exe"
    WriteINIStr "$SMPROGRAMS\$StartMenuFolder\The ${MY_PRODUCT} HomePage.url" "InternetShortcut" "URL" "${PRODUCT_HOMEPAGE}"

 !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;-------------------------------------------------------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecDigiKam ${LANG_ENGLISH} "This is the digiKam for Windows including kipi-plugins and KDE 4.4."
  LangString DESC_SecDigiKam ${LANG_FRENCH} "digiKam pour Windows incluant les greffons kipi et KDE 4.4."
  LangString DESC_SecDigiKam ${LANG_GERMAN} "Dies ist die digiKam für Windows einschließlich kipi-Plugins und KDE 4.4."
  LangString DESC_SecDigiKam ${LANG_SPANISH} "Este es el digiKam para Windows, incluyendo kipi-plugins y KDE 4.4."
  LangString DESC_SecDigiKam ${LANG_ITALIAN} "Questo è il digiKam per Windows, inclusi Kipi-plugins e KDE 4.4."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDigiKam} $(DESC_SecDigiKam)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;-------------------------------------------------------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"

  RMDir /r "$INSTDIR\kde4"

  RMDir "$INSTDIR"

  ;remove start menu items
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${MY_PRODUCT}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Showfoto.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\The ${MY_PRODUCT} HomePage.url"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey /ifempty HKCU "Software\${MY_PRODUCT}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}"

SectionEnd
