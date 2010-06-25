@echo off

set KDE4_INSTALL_DIR=E:\kde4-msvc
set PATH=%PATH%;%KDE4_INSTALL_DIR%\bin;%KDE4_INSTALL_DIR%\plugins
set KDE4_INCLUDE_DIR=%KDE4_INSTALL_DIR%\include
set KDEDIRS=%KDE4_INSTALL_DIR%
set KDEROOT=%KDE4_INSTALL_DIR%
set QTMAKESPEC=%KDE4_INSTALL_DIR%\mkspecs\win32-g++
set QT_PLUGIN_PATH=%KDE4_INSTALL_DIR%\plugins
set QTDIR=%KDE4_INSTALL_DIR%
set QT_INSTALL_DIR=%KDE4_INSTALL_DIR%
set SVN_EDITOR="E:\mc\mc -e"

call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"

call "E:\mc\mc.exe"