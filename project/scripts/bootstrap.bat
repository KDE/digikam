@ECHO OFF

REM Adjust there the right path to KDE4 install directory.
SET KDEDIRS=E:\kde4-mingw

SET PATH=%PATH%;%KDEDIRS%\bin;%KDEDIRS%\plugins
SET KDE4_INSTALL_DIR=%KDEDIRS%
SET KDEDIRS=%KDEDIRS%
SET KDE4_LIB_DIRS=%KDEDIRS%
SET KDEROOT=%KDEDIRS%
SET QTMAKESPEC=%KDEDIRS%\mkspecs\win32-g++
SET QT_PLUGIN_PATH=%KDEDIRS%\plugins
SET QTDIR=%KDEDIRS%
SET QT_INSTALL_DIR=%KDEDIRS%

REM Clean-up CMake cache file.
DEL /F CMakeCache.txt

REM configure CMake env with right compiler. Uncoment the right line.

REM MinGW : gcc for Windows.
cmake -G "MinGW Makefiles" . -DCMAKE_INCLUDE_PATH=%KDEDIRS%/include -DCMAKE_LIBRARY_PATH=%KDEDIRS%/lib -DCMAKE_INSTALL_PREFIX=%KDE4_INSTALL_DIR%

REM Microsoft Visual C++ IDE.
REM cmake -G "Visual Studio 9 2008" . -DEXPAT_LIBRARY=%KDEDIRS%/lib/libexpat.lib -DCMAKE_INCLUDE_PATH=%KDEDIRS%/include -DCMAKE_LIBRARY_PATH=%KDEDIRS%/lib -DCMAKE_INSTALL_PREFIX=%KDE4_INSTALL_DIR%

REM Microsoft Visual C++ command line compiler.
REM cmake -G "NMake Makefiles" . -DEXPAT_LIBRARY=%KDEDIRS%/lib/libexpat.lib -DCMAKE_INCLUDE_PATH=%KDEDIRS%/include -DCMAKE_LIBRARY_PATH=%KDEDIRS%/lib -DCMAKE_INSTALL_PREFIX=%KDE4_INSTALL_DIR%

