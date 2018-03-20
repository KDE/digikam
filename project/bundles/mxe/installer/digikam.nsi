;; ============================================================
 ;
 ; This file is a part of digiKam project
 ; http://www.digikam.org
 ;
 ; Date        : 2010-11-08
 ; Description : Null Soft Windows Installer based for digiKam.
 ;               Note: NSIS >= 3 is required to be compatible with Windows 10.
 ;
 ; Copyright (C) 2010      by Julien Narboux <julien at narboux dot fr>
 ; Copyright (C) 2011-2014 by Ananta Palani  <anantapalani at gmail dot com>
 ; Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 ;
 ; Script arguments:
 ; VERSION    : the digiKam version as string.
 ; BUNDLEPATH : the path where whole digiKam bundle is installed.
 ; TARGETARCH : the target Windows architecture (32 or 64 bits).
 ; OUTPUT     : the output installer file name as string.
 ;
 ; Example: makensis -DVERSION=5.0.0 -DTARGETARCH=32 -DBUNDLEPATH=../bundle digikam.nsi
 ;
 ; Extra NSIS plugin to install in order to run this script :
 ;
 ; Registry   : http://nsis.sourceforge.net/Registry_plug-in
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
;Verbose level (no script)

    !verbose 3

;-------------------------------------------------------------------------------
; Compression rules optimizations
; We will use LZMA compression as 7Zip, with a dictionary size of 96Mb (to reproduce 7Zip Ultra compression mode)

    SetCompress force
    SetCompressor /SOLID lzma
    SetDatablockOptimize on
    SetCompressorDictSize 96

;-------------------------------------------------------------------------------
;Include Modern UI

    ; Turn on UTF-8 strings encoding support.
    Unicode true

    ; Add support to hdpi screen. This require NSIS >= 3.x
    ManifestDPIAware true

    !include "MUI2.nsh"
    !define MY_PRODUCT "digiKam"
    !define PRODUCT_HOMEPAGE "https://www.digikam.org"
    !define SUPPORT_HOMEPAGE "https://www.digikam.org/support"
    !define ABOUT_HOMEPAGE "https://www.digikam.org/about"
    !define OUTFILE "${OUTPUT}"

;-------------------------------------------------------------------------------
;General Setup

    ;Name and file

    Name "${MY_PRODUCT} ${VERSION}"
    Icon "digikam-installer.ico"
    UninstallIcon "digikam-uninstaller.ico"
    OutFile "${OUTFILE}"

    ;Request application privileges for Windows Vista

    RequestExecutionLevel admin

    !include x64.nsh

    ;Get installation folder from registry if available

    InstallDirRegKey HKLM "Software\${MY_PRODUCT}" ""

    !include "LogicLib.nsh"
    !include "StrFunc.nsh"
    ${StrRep}
    ${StrStr}
    ${StrStrAdv}

    ;Requires Registry plugin :
    ;http://nsis.sourceforge.net/Registry_plug-in

    !addplugindir "./plugins"
    !include "./plugins/Registry.nsh"

    ;Extra local scripts

    !include "process_running.nsh"
    !include "reboot_required.nsh"
    !include "readme_page.nsh"
    !include "events_functions.nsh"

;-------------------------------------------------------------------------------
;Interface Configuration

    !define MUI_HEADERIMAGE
    !define MUI_HEADERIMAGE_BITMAP "digikam_header.bmp" 
    !define MUI_WELCOMEFINISHPAGE_BITMAP "digikam_welcome.bmp"
    !define MUI_UNWELCOMEFINISHPAGE_BITMAP "digikam_welcome.bmp"
    !define MUI_ABORTWARNING
    !define MUI_ICON "digikam-installer.ico"
    !define MUI_UNICON "digikam-uninstaller.ico"
    !define !define MUI_FINISHPAGE_RUN
    !define MUI_FINISHPAGE_RUN
    !define MUI_FINISHPAGE_RUN_TEXT "Read release notes"
    !define MUI_FINISHPAGE_RUN_FUNCTION functionFinishRun
    !define MUI_FINISHPAGE_LINK "Visit digiKam project website"
    !define MUI_FINISHPAGE_LINK_LOCATION "https://www.digikam.org"

;-------------------------------------------------------------------------------
;Page Definitions

    ;Variable for the folder of the start menu

    Var StartMenuFolder

    !insertmacro MUI_PAGE_WELCOME
    !insertmacro MUI_PAGE_README "ABOUT.txt"
    !insertmacro MUI_PAGE_LICENSE "GPL.txt"
    !define MUI_PAGE_CUSTOMFUNCTION_LEAVE DirectoryLeave
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
;Languages List

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

        ;Check running processes prior to install

        Call CheckDigikamRunning
        Call CheckShowfotoRunning

        SetOutPath "$INSTDIR"

        File "../data/releasenotes.html"
        File "digikam-uninstaller.ico"

        ;Copy only required directories
        ;The SetOutPath is required because otherwise NSIS will assume all files are
        ;in the same folder even though they are sourced from different folders
        ;The \*.* is required for File /r because without it, NSIS would add every
        ;folder with the name 'bin' in all subdirectories of ${BUNDLEPATH}

        SetOutPath "$INSTDIR\"
        File /r "${BUNDLEPATH}\*.dll"
        File /r "${BUNDLEPATH}\*.exe"
        File /r "${BUNDLEPATH}\*.conf"
        File /r "${BUNDLEPATH}\*.rcc"

        SetOutPath "$INSTDIR\data"
        File /r "${BUNDLEPATH}\data\*.*"

        SetOutPath "$INSTDIR\plugins"
        File /r "${BUNDLEPATH}\plugins\*.*"

        SetOutPath "$INSTDIR\translations"
        File /r "${BUNDLEPATH}\translations\*.*"

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
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "Readme" '"$INSTDIR\releasenotes.html"'
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "URLInfoAbout" "${ABOUT_HOMEPAGE}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "URLUpdateInfo" "${PRODUCT_HOMEPAGE}"

        ;Calculate the install size so that it can be shown in the uninstall interface in Windows
        ;see http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs
        ;this isn't the most accurate method but it is very fast and is accurate enough for an estimate

        push $0
        SectionGetSize ${SecDigiKam} $0
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "EstimatedSize" "$0"
        pop $0

        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "NoModify" "1"
        WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}" "NoRepair" "1"

        ;Add start menu items to All Users

        SetShellVarContext all
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

        ;Create shortcuts

        CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
        SetOutPath "$INSTDIR\bin"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${MY_PRODUCT}.lnk" "$INSTDIR\digikam.exe"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Showfoto.lnk" "$INSTDIR\showfoto.exe"

        WriteINIStr "$SMPROGRAMS\$StartMenuFolder\The ${MY_PRODUCT} HomePage.url" "InternetShortcut" "URL" "${PRODUCT_HOMEPAGE}"

        !insertmacro MUI_STARTMENU_WRITE_END

    SectionEnd

;-------------------------------------------------------------------------------
;Uninstaller Section

    Section "Uninstall"

        ;Check running processes prior to uninstall

        Call un.CheckDigikamRunning
        Call un.CheckShowfotoRunning

        Delete "$INSTDIR\Uninstall.exe"
        Delete "$INSTDIR\releasenotes.html"
        Delete "$INSTDIR\digikam-uninstaller.ico"

        RMDir /r "$INSTDIR\"
        RMDir /r "$INSTDIR\data"
        RMDir /r "$INSTDIR\plugins"
        RMDir /r "$INSTDIR\translations"

        ;Do not do a recursive removal of $INSTDIR because user may have accidentally installed into system critical directory!

        RMDir "$INSTDIR"

        ;Remove start menu items

        SetShellVarContext all
        !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

        Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
        Delete "$SMPROGRAMS\$StartMenuFolder\${MY_PRODUCT}.lnk"
        Delete "$SMPROGRAMS\$StartMenuFolder\Showfoto.lnk"
        Delete "$SMPROGRAMS\$StartMenuFolder\The ${MY_PRODUCT} HomePage.url"

        RMDir /r "$SMPROGRAMS\$StartMenuFolder"

        ;Remove registry entries

        DeleteRegKey /ifempty HKLM "Software\${MY_PRODUCT}"
        DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MY_PRODUCT}"

    SectionEnd
