# Script to build QtAV for digiKam bundle.
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(PREFIX_ext_qtav "${EXTPREFIX}")
set(QTAV_VERSION v1.12.0)

if(MINGW)

    if (${MXE_ARCHBITS} MATCHES "64")
        set(LIB_DIR "lib_win_x86_64")
    else()
        set(LIB_DIR "lib_win_x86")
    endif()

    ExternalProject_Add(ext_qtav

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        GIT_REPOSITORY https://github.com/wang-bin/QtAV.git
        GIT_TAG ${QTAV_VERSION}

        CONFIGURE_COMMAND rm -rfv ${MXE_INSTALL_PREFIX}/qt5/include/QtAV &&
                          rm -rfv ${MXE_INSTALL_PREFIX}/qt5/include/QtAVWidgets &&
                          rm -fv  ${MXE_INSTALL_PREFIX}/qt5/lib/libQtAV1.a &&
                          rm -fv  ${MXE_INSTALL_PREFIX}/qt5/lib/libQtAVWidgets1.a &&
                          rm -fv  ${MXE_INSTALL_PREFIX}/qt5/bin/QtAV1.dll &&
                          rm -fv  ${MXE_INSTALL_PREFIX}/qt5/bin/QtAVWidgets1.dll &&
                          mkdir build && cd build && ${MXE_INSTALL_PREFIX}/qt5/bin/qmake CONFIG+=no_examples ..

        BUILD_COMMAND cd build && $(MAKE)

        INSTALL_COMMAND cp -rfv <SOURCE_DIR>/src/QtAV ${MXE_INSTALL_PREFIX}/qt5/include/ &&
                        cp -rfv <SOURCE_DIR>/widgets/QtAVWidgets ${MXE_INSTALL_PREFIX}/qt5/include/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAV1.a ${MXE_INSTALL_PREFIX}/qt5/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAVWidgets1.a ${MXE_INSTALL_PREFIX}/qt5/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/QtAV1.dll ${MXE_INSTALL_PREFIX}/qt5/bin/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/QtAVWidgets1.dll ${MXE_INSTALL_PREFIX}/qt5/bin/

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

elseif(APPLE)

    ExternalProject_Add(ext_qtav

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        GIT_REPOSITORY https://github.com/wang-bin/QtAV.git
        GIT_TAG ${QTAV_VERSION}

        PATCH_COMMAND rm -rfv ${INSTALL_ROOT}/libexec/qt5/lib/QtAV.framework &&
                      rm -rfv ${INSTALL_ROOT}/libexec/qt5/lib/QtAVWidgets.framework &&
                      rm -rfv ${INSTALL_ROOT}/libexec/qt5/include/QtAV &&
                      rm -rfv ${INSTALL_ROOT}/libexec/qt5/include/QtAVWidgets &&
                      ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qtav-rules-macos.patch

        CONFIGURE_COMMAND mkdir build && cd build && qmake ..

        BUILD_COMMAND cd build && $(MAKE)

        INSTALL_COMMAND install_name_tool -id ${INSTALL_ROOT}/libexec/qt5/lib/QtAV.framework/Versions/1/QtAV <SOURCE_DIR>/build/lib_osx_x86_64_llvm/QtAV.framework/QtAV &&
                        install_name_tool -id ${INSTALL_ROOT}/libexec/qt5/lib/QtAVWidgets.framework/Versions/1/QtAVWidgets <SOURCE_DIR>/build/lib_osx_x86_64_llvm/QtAVWidgets.framework/QtAVWidgets &&
                        install_name_tool -change QtAV.framework/Versions/1/QtAV ${INSTALL_ROOT}/libexec/qt5/lib/QtAV.framework/Versions/1/QtAV <SOURCE_DIR>/build/lib_osx_x86_64_llvm/QtAVWidgets.framework/QtAVWidgets &&
                        cp -rfv <SOURCE_DIR>/build/lib_osx_x86_64_llvm/QtAV.framework ${INSTALL_ROOT}/libexec/qt5/lib/ &&
                        cp -rfv <SOURCE_DIR>/build/lib_osx_x86_64_llvm/QtAVWidgets.framework ${INSTALL_ROOT}/libexec/qt5/lib/ &&
                        ln -s ${INSTALL_ROOT}/libexec/qt5/lib/QtAV.framework/Headers ${INSTALL_ROOT}/libexec/qt5/include/QtAV &&
                        ln -s ${INSTALL_ROOT}/libexec/qt5/lib/QtAVWidgets.framework/Headers ${INSTALL_ROOT}/libexec/qt5/include/QtAVWidgets

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

else() #Linux

    set(LIB_DIR "lib_linux_")

    ExternalProject_Add(ext_qtav

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        GIT_REPOSITORY https://github.com/wang-bin/QtAV.git
        GIT_TAG ${QTAV_VERSION}

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qtav-rules-appimage.patch

        CONFIGURE_COMMAND rm -rfv ${INSTALL_ROOT}/include/QtAV &&
                          rm -rfv ${INSTALL_ROOT}/include/QtAVWidgets &&
                          rm -fv  ${INSTALL_ROOT}/lib/libQtAV.so &&
                          rm -fv  ${INSTALL_ROOT}/lib/libQtAV.so.1 &&
                          rm -fv  ${INSTALL_ROOT}/lib/libQtAV.so.1.12 &&
                          rm -fv  ${INSTALL_ROOT}/lib/libQtAV.so.1.12.0 &&
                          rm -fv  ${INSTALL_ROOT}/lib/libQtAVWidgets.so &&
                          rm -fv  ${INSTALL_ROOT}/lib/libQtAVWidgets.so.1 &&
                          rm -fv  ${INSTALL_ROOT}/lib/libQtAVWidgets.so.1.12 &&
                          rm -fv  ${INSTALL_ROOT}/lib/libQtAVWidgets.so.1.12.0 &&
                          mkdir build && cd build && qmake ..

        BUILD_COMMAND cd build && $(MAKE)

        INSTALL_COMMAND cp -rfv <SOURCE_DIR>/src/QtAV ${INSTALL_ROOT}/include/ &&
                        cp -rfv <SOURCE_DIR>/widgets/QtAVWidgets ${INSTALL_ROOT}/include/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAV.so ${INSTALL_ROOT}/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAV.so.1 ${INSTALL_ROOT}/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAV.so.1.12 ${INSTALL_ROOT}/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAV.so.1.12.0 ${INSTALL_ROOT}/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAVWidgets.so ${INSTALL_ROOT}/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAVWidgets.so.1 ${INSTALL_ROOT}/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAVWidgets.so.1.12 ${INSTALL_ROOT}/lib/ &&
                        cp -fv  <SOURCE_DIR>/build/${LIB_DIR}/libQtAVWidgets.so.1.12.0 ${INSTALL_ROOT}/lib/

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
