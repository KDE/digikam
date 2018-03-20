;; ============================================================
 ;
 ; This file is a part of digiKam project
 ; http://www.digikam.org
 ;
 ; Date        : 2010-11-08
 ; Description : Functions to check if reboot is required.
 ;               Note: NSIS >= 3 is required to be compatible with Windows 10.
 ;
 ; Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

!ifndef REBOOT_REQUIRED_INCLUDED
!define REBOOT_REQUIRED_INCLUDED

Function DirectoryLeave

    Call NotifyIfRebootRequired

FunctionEnd

;-------------------------------------------

Function NotifyIfRebootRequired

    Call IsRebootRequired
    Exch $0

    ${If} $0 == 1

        ;TODO: consider adding a RunOnce entry for the installer to HKCU instead of telling the user they need to run the installer
        ;themselves (can't add to HKLM because basic user wouldn't have access, only admins do).
        ;this would require using the UAC plugin to handle elevation by starting as a normal user, elevating, and then dropping back to normal when writing to HKCU
        ;TODO: need to internationalize string (see VLC / clementine / etc)

        MessageBox MB_YESNO|MB_ICONSTOP|MB_TOPMOST|MB_SETFOREGROUND "You must reboot to complete uninstallation of a previous install of ${MY_PRODUCT} before ${MY_PRODUCT} ${VERSION} can be installed.$\r$\n$\r$\n\
            Would you like to reboot now?$\r$\n$\r$\n\
            (You will have to run the installer again after reboot to continue setup)" /SD IDNO IDNO noInstall
            Reboot

    ${Else}

        Goto done

    ${EndIf}

    noInstall:
        Abort

    done:
        Pop $0

FunctionEnd

;-------------------------------------------------------------------------------------
;This function require Registry plugin (http://nsis.sourceforge.net/Registry_plug-in)

Function IsRebootRequired

    Push $0
    Push $1
    Push $2
    Push $3

    ${registry::Read} "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager" "PendingFileRenameOperations" $0 $1
    ${registry::Unload}

    ${If} $0 != ""

        StrLen $2 "$INSTDIR"
        ${StrStr} $1 "$0" "$INSTDIR"
        StrCpy $3 $1 $2
        ${AndIf} $3 == "$INSTDIR"
        StrCpy $0 1

    ${Else}

        StrCpy $0 0

    ${EndIf}

    Pop $3
    Pop $2
    Pop $1
    Exch $0

FunctionEnd

!endif ;REBOOT_REQUIRED_INCLUDED
