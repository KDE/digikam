# Script to build Lensfun for digiKam bundle.
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_lensfun "${EXTPREFIX}" )

if (MINGW)

    ExternalProject_Add(ext_lensfun

        URL file://${EXTERNALS_DOWNLOAD_DIR}/lensfun-0.3.95.tar.gz
        URL_MD5 21107eaf72303706256481fef2dc8013

        CONFIGURE_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.mxe <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.mxe ${MXE_BUILDROOT} RelWithDebInfo ${GLOBAL_PROFILE}

        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

    ExternalProject_Add_step(ext_lensfun before_download

        # Note : under MXE, curl is not compiled with https support. We cannot use the tarball as under Linux/MacOS.
        COMMAND wget -N http://downloads.sourceforge.net/project/lensfun/0.3.95/lensfun-0.3.95.tar.gz -O ${EXTERNALS_DOWNLOAD_DIR}/lensfun-0.3.95.tar.gz
        DEPENDERS download
    )

else()

    ExternalProject_Add(ext_lensfun

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL http://downloads.sourceforge.net/project/lensfun/0.3.95/lensfun-0.3.95.tar.gz
        URL_MD5 21107eaf72303706256481fef2dc8013

        INSTALL_DIR ${EXTPREFIX_lensfun}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_lensfun}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
