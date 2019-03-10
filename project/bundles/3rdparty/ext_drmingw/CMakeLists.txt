# Script to install DrMinGw run-time for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(PREFIX_ext_drmingw "${EXTPREFIX}")

ExternalProject_Add(ext_drmingw
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/jrfonseca/drmingw/archive/0.9.1.tar.gz
    URL_MD5 fdafc515b83adcca765dc5cae05f3828

    INSTALL_DIR ${EXTPREFIX_drmingw}

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/drmingw-coreonly.patch

    CONFIGURE_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.mxe <SOURCE_DIR>/. &&
                      <SOURCE_DIR>/bootstrap.mxe ${MXE_BUILDROOT} RelWithDebInfo -DBUILD_TESTING=OFF

    BUILD_COMMAND cd <SOURCE_DIR>/build.mxe && $(MAKE)

    INSTALL_COMMAND cd <SOURCE_DIR>/build.mxe && make install/fast

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
