;; ============================================================
 ;
 ; This file is a part of digiKam project
 ; http://www.digikam.org
 ;
 ; Date        : 2005-01-01
 ; Description : Functions to check if executable is running.
 ;               Note: NSIS >= 3 is required to be compatible with Windows 10.
 ;
 ; Copyright (C) 2005-2017 by Tim Kosse <tim dot kosse at filezilla-project dot org>
 ; Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

!ifndef EXECUTABLE_RUNNING_INCLUDED
!define EXECUTABLE_RUNNING_INCLUDED

!include "LogicLib.nsh"

; Returns number of processes * 4 on top of stack,
; array with all processes right below it.
; Array should be cleared using System::Free

!macro ENUMPROCESSES un

    Function ${un}EnumProcesses

        ; Is this really necesssary for $Rx?
        Push $R1
        Push $R0
        Push $R2
        Push $R3

        ; Double size of array each time EnumProcesses fills it completely so that
        ; we do get all processes

        StrCpy $R1 1024

        enum_processes_loop:

        System::Alloc $R1

        Pop $R0

        System::Call "psapi::EnumProcesses(i R0, i R1, *i .R2) i .R3"

        ${If} $R3 == 0
            ; EnumProcesses failed, how can that be? :P
            goto enum_processes_fail
        ${EndIf}

        ${If} $R1 == $R2

            ; Too small buffer. Retry with twice the size

            Intop $R1 $R1 * 2
            System::Free $R0

            goto enum_processes_loop

        ${EndIf}

        StrCpy $R1 $R2

        ; Restore registers
        ; and put results on stack
        Pop $R3
        Pop $R2
        Exch $R0
        Exch
        Exch $R1
        return

        enum_processes_fail:

        Pop $R3
        Pop $R2
        Pop $R0
        Pop $R1

        Push 0
        Push 0

    FunctionEnd

!macroend

; Insert function as an installer and uninstaller function.
!insertmacro ENUMPROCESSES ""
!insertmacro ENUMPROCESSES "un."

;-------------------------------------------

; Expects process ID on top of stack, returns
; filename (in device syntax) on top of stack

!macro GETFILENAMEFROMPROCESSID un

    Function ${un}GetFilenameFromProcessId

        Exch $R0
        Push $R1
        Push $R2
        Push $R3

        !define ${un}PROCESS_QUERY_INFORMATION 0x0400
        System::Call "kernel32::OpenProcess(i ${${un}PROCESS_QUERY_INFORMATION}, i 0, i $R0) i .R0"

        ${If} $R0 == 0

            Pop $R3
            Pop $R2
            Pop $R1
            Pop $R0
            Push ''
            return

        ${EndIf}

        StrCpy $R3 ${NSIS_MAX_STRLEN}
        System::Call "kernel32::QueryFullProcessImageName(i R0, i 0, t .R1, *i R3) i .R2"

        ${If} $R2 == 0
            ; Fallback
            System::Call "psapi::GetProcessImageFileName(i R0, t .R1, i ${NSIS_MAX_STRLEN}) i .R2"
        ${EndIf}

        ${If} $R2 == 0

            System::Call "kernel32::CloseHandle(i R0)"
            Pop $R3
            Pop $R2
            Pop $R1
            Pop $R0
            Push ''

            return

        ${EndIf}

        System::Call "kernel32::CloseHandle(i R0)"

        Pop $R3
        Pop $R2
        StrCpy $R0 $R1
        Pop $R1
        Exch $R0

    FunctionEnd

!macroend

; Insert function as an installer and uninstaller function.
!insertmacro GETFILENAMEFROMPROCESSID ""
!insertmacro GETFILENAMEFROMPROCESSID "un."

;-------------------------------------------

; Expects process name on top of stack. Afterwards, top of stack contains path
; to the process if it's running or an empty string if it is not.

!macro ISPROCESSRUNNING un

    Function ${un}IsProcessRunning

        Exch $R0 ; Name

        Push $R1 ; Bytes
        Push $R2 ; Array
        Push $R3 ; Counter
        Push $R4 ; Strlen
        Push $R5 ; Current process ID and image filename
        Push $R6 ; Last part of path

        StrCpy $R0 "\$R0"

        StrLen $R4 $R0
        IntOp $R4 0 - $R4

        Call ${un}EnumProcesses

        Pop $R1
        Pop $R2

        StrCpy $R3 0

        ${While} $R3 < $R1

            IntOp $R5 $R2 + $R3

            System::Call "*$R5(i .R5)"
            Push $R5

            Call ${un}GetFilenameFromProcessId

            Pop $R5

            ; Get last part of filename
            StrCpy $R6 $R5 '' $R4

            ${If} $R6 == $R0

            ; Program is running
            StrCpy $R0 $R5
            Pop $R6
            Pop $R5
            Pop $R4
            Pop $R3
            Pop $R2
            Pop $R1
            Exch $R0
            return

            ${EndIf}

            IntOp $R3 $R3 + 4

        ${EndWhile}

        Pop $R5
        Pop $R4
        Pop $R3
        Pop $R2
        Pop $R1
        Pop $R0
        Push ''

    FunctionEnd

!macroend

; Insert function as an installer and uninstaller function.
!insertmacro ISPROCESSRUNNING ""
!insertmacro ISPROCESSRUNNING "un."

;-------------------------------------------

!macro CHECKDIGIKAM un

    Function ${un}CheckDigikamRunning

        Push "digikam.exe"
        Call ${un}IsProcessRunning
        Pop $R1

        ${While} $R1 != ''

            MessageBox MB_ABORTRETRYIGNORE|MB_DEFBUTTON2 "digiKam appears to be running.$\nPlease close all running instances of digiKam before continuing the installation." /SD IDIGNORE IDABORT CheckDigikamRunning_abort IDIGNORE CheckDigikamRunning_ignore

            Push "digikam.exe"
            Call ${un}IsProcessRunning
            Pop $R1

        ${EndWhile}

        CheckDigikamRunning_ignore:
        Return

        CheckDigikamRunning_abort:
        Quit

    FunctionEnd

!macroend

; Insert function as an installer and uninstaller function.
!insertmacro CHECKDIGIKAM ""
!insertmacro CHECKDIGIKAM "un."

;-------------------------------------------

!macro CHECKSHOWFOTO un

    Function ${un}CheckShowfotoRunning

        Push "showfoto.exe"
        Call ${un}IsProcessRunning
        Pop $R1

        ${While} $R1 != ''

            MessageBox MB_ABORTRETRYIGNORE|MB_DEFBUTTON2 "Showfoto appears to be running.$\nPlease close all running instances of Showfoto before continuing the installation." /SD IDIGNORE IDABORT CheckShowfotoRunning_abort IDIGNORE CheckShowfotoRunning_ignore

            Push "showfoto.exe"
            Call ${un}IsProcessRunning
            Pop $R1

        ${EndWhile}

        CheckShowfotoRunning_ignore:
        Return

        CheckShowfotoRunning_abort:
        Quit

    FunctionEnd

!macroend

; Insert function as an installer and uninstaller function.
!insertmacro CHECKSHOWFOTO ""
!insertmacro CHECKSHOWFOTO "un."

!endif ;EXECUTABLE_RUNNING_INCLUDED
