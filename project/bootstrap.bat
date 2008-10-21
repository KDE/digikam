@echo off
rem KDE4 must be installed into C:\KDE4
set KDEDIRS=C:\KDE4
set PATH=%PATH%;%KDEDIRS%\bin

rem clean up CMake cache
del -f CMakeCache.txt

rem configure CMake env.
cmake -G "MinGW Makefiles" . -DCMAKE_LIBRARY_PATH=%KDEDIRS%/lib -DENABLE_GPHOTO2=OFF -DCMAKE_INSTALL_PREFIX=C:/KDE4
