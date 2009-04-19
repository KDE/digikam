@echo off

set KDEDIRS=E:\kde4-mingw
set PATH=%PATH%;%KDEDIRS%\bin
set QT_PLUGIN_PATH=%KDEDIR%\plugins
set KDE4_INSTALL_DIR=%KDEDIRS%
set QT_INSTALL_DIR=%KDEDIRS%

rem clean up CMake cache
del /F CMakeCache.txt

rem configure CMake env.
cmake -G "MinGW Makefiles" . -DCMAKE_LIBRARY_PATH=%KDEDIRS%/lib -DENABLE_GPHOTO2=OFF -DCMAKE_INSTALL_PREFIX=%KDE4_INSTALL_DIR% 
rem cmake -G "Visual Studio 9 2008" . -DEXPAT_LIBRARY=%KDEDIRS%/lib/libexpat.lib  -DCMAKE_LIBRARY_PATH=%KDEDIRS%/lib -DENABLE_GPHOTO2=OFF -DCMAKE_INSTALL_PREFIX=%KDE4_INSTALL_DIR% 
