@echo off

set KDE4_INSTALL_DIR=C:\KDE
set PATH=%PATH%;%KDE4_INSTALL_DIR%\bin;%KDE4_INSTALL_DIR%\plugins
set KDE4_INCLUDE_DIR=%KDE4_INSTALL_DIR%\include
set KDEDIRS=%KDE4_INSTALL_DIR%
set KDEROOT=%KDE4_INSTALL_DIR%
set QTMAKESPEC=%KDE4_INSTALL_DIR%\mkspecs\win32-g++
set QT_PLUGIN_PATH=%KDE4_INSTALL_DIR%\plugins
set QTDIR=%KDE4_INSTALL_DIR%
set QT_INSTALL_DIR=%KDE4_INSTALL_DIR%
set GIT_EDITOR="C:\KDE\bin\kwrite"
set CL=/MP

call "C:\Program Files\Microsoft Visual Studio 10.0\Common7\Tools\vsvars32.bat"

call "C:\Devel\mc\mc.exe"