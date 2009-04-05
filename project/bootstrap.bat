@echo off

rem KDE4 must be installed into C:\KDE4
set KDEDIRS=C:\KDE4
set PATH=%PATH%;%KDEDIRS%\bin
set QT_PLUGIN_PATH=%KDEDIRS%\plugins
set KDE4_INSTALL_DIR=%KDEDIRS%

rem clean up CMake cache
del /F "CMakeCache.txt"

rem configure CMake env.

rem We will work on command line using MinGW compiler
rem set MAKEFILES_TYPE="MinGW Makefiles"

rem We will work with CodeBlock IDE using MinGW compiler
set MAKEFILES_TYPE="CodeBlocks - MinGW Makefiles"

cmake -G %MAKEFILES_TYPE% . -DCMAKE_LIBRARY_PATH=%KDEDIRS%/lib -DENABLE_GPHOTO2=OFF -DCMAKE_INSTALL_PREFIX=%KDE4_INSTALL_DIR%
