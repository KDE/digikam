# Script to install DrMinGw run-time for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(PREFIX_ext_drmingw "${EXTPREFIX}")
set(DRMINGW_VERSION 0.9.1)

if (MINGW)

    # Rules to extract and install DrMingw runtime and DbgHelp binaries dependencies from official DrMingw binaries tarball.

    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

        # 64 bits system

        ExternalProject_Add(ext_drmingw
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/${DRMINGW_VERSION}/drmingw-${DRMINGW_VERSION}-win64.7z
            URL_MD5 979c32f1a5190fc20adc04dbd8b9743a

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. Mingw runtime 64-bits and DbgHelp dependencies

            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll  ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes  ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/exchndl.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/mgwhelp.dll ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )

    else()

        # 32 bits system

        ExternalProject_Add(ext_drmingw
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://github.com/jrfonseca/drmingw/releases/download/${DRMINGW_VERSION}/drmingw-${DRMINGW_VERSION}-win32.7z
            URL_MD5 1de4e89429a3a6a9debd3c6bb2bd3ea1

            INSTALL_DIR ${PREFIX_ext_drmingw}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Dr. Mingw runtime 32-bits and DbgHelp dependencies

            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/dbghelp.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.dll  ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/symsrv.yes  ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/exchndl.dll ${PREFIX_ext_drmingw}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/mgwhelp.dll ${PREFIX_ext_drmingw}/bin/

            UPDATE_COMMAND ""
        )

    endif()

endif()
