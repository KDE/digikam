# Script to build libksane for digiKam bundle.
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(EXTPREFIX_libksane "${EXTPREFIX}" )

set(KA_VERSION 17.12.1)

if(MINGW)

    ExternalProject_Add(ext_libksane
                        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                        URL http://download.kde.org/stable/applications/${KA_VERSION}/src/libksane-${KA_VERSION}.tar.xz
                        URL_MD5 6b43e4e63e503a49f99f081df8cbf4d2

                        INSTALL_DIR ${EXTPREFIX_libksane}
                        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libksane-drop-ktextwidgets.patch

                        CMAKE_ARGS -DMXE_TOOLCHAIN=${MXE_TOOLCHAIN}
                                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                   -DCMAKE_COLOR_MAKEFILE=ON
                                   -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_libksane}
                                   -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
                                   -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN}
                                   -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                                   -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                                   -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include
                                   -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib
                                   -DZLIB_ROOT=${CMAKE_PREFIX_PATH}
                                   ${GLOBAL_PROFILE}
                                   "${argoptions}"
                                   -DWITH_DESIGNER_PLUGIN=OFF
                                   -DBUILD_TESTING=OFF
                                   -Wno-dev

                        UPDATE_COMMAND ""
                        ALWAYS 0
    )

elseif(APPLE)

    ExternalProject_Add(ext_libksane
                        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                        URL http://download.kde.org/stable/applications/${KA_VERSION}/src/libksane-${KA_VERSION}.tar.xz
                        URL_MD5 6b43e4e63e503a49f99f081df8cbf4d2

                        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libksane-drop-ktextwidgets.patch

                        CONFIGURE_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                                          <SOURCE_DIR>/bootstrap.macports ${EXTPREFIX_libksane} debug x86_64 "-DWITH_DESIGNER_PLUGIN=OFF -DBUILD_TESTING=OFF -Wno-dev"

                        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

                        INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

                        BUILD_IN_SOURCE 1

                        UPDATE_COMMAND ""
                        ALWAYS 0
        )

else() # Linux

    ExternalProject_Add(ext_libksane
                        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                        URL http://download.kde.org/stable/applications/${KA_VERSION}/src/libksane-${KA_VERSION}.tar.xz
                        URL_MD5 6b43e4e63e503a49f99f081df8cbf4d2
                        INSTALL_DIR ${EXTPREFIX_libksane}
                        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_libksane}
                                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                                   ${GLOBAL_PROFILE}
                                   -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX}
                                   -DWITH_DESIGNER_PLUGIN=OFF
                                   -DBUILD_TESTING=OFF
                                   -Wno-dev
                        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libksane-drop-ktextwidgets.patch
                        UPDATE_COMMAND ""
                        ALWAYS 0
   )

endif()
