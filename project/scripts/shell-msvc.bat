@echo off

set KDEDIRS=E:\kde4-msvc
set PATH=%PATH%;%KDEDIRS%\bin;%KDEDIRS%\plugins
set KDE4_INSTALL_DIR=%KDEDIRS%
set KDEDIRS=%KDEDIRS%
set KDEROOT=%KDEDIRS%
set QTMAKESPEC=%KDEDIRS%\mkspecs\win32-g++
set QT_PLUGIN_PATH=%KDEDIRS%\plugins
set QTDIR=%KDEDIRS%
set QT_INSTALL_DIR=%KDEDIRS%
set SVN_EDITOR="C:\mc\mc -e"


call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
@set PATH=C:\Program Files\Microsoft Visual Studio 9.0\VC\bin;%PATH% 
@set INCLUDE=C:\Program Files\Microsoft Visual Studio 9.0\VC\include;%INCLUDE% 
@set LIB=C:\Program Files\Microsoft Visual Studio 9.0\VC\lib;%LIB%

C:\mc\mc.exe