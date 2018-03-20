# Script to build Hugin for digiKam bundle.
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(PREFIX_ext_hugin "${EXTPREFIX}")

set(Hugin_Conf -DDISABLE_DPKG=ON
               -DBUILD_HSI=OFF
               -DENABLE_LAPACK=OFF
               -DBUILD_HUGINSETUP=OFF
)

JoinListAsString("${Hugin_Conf}" " " BASH_OPTIONS)

if(MINGW)

    ExternalProject_Add(ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        # To use official release tarball

        #URL http://www.exiv2.org/exiv2-0.25.tar.gz
        #URL_MD5 258d4831b30f75a01e0234065c6c2806

        # To use current implementation from SVN

        SVN_REPOSITORY svn://dev.exiv2.org/svn/trunk

        CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.mxe ${MXE_BUILDROOT} RelWithDebInfo ${BASH_OPTIONS}

        BUILD_COMMAND cd <SOURCE_DIR>/build && make -j

        INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )


elseif(APPLE)

    ExternalProject_Add(ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        # To use official release tarball

        #URL http://www.exiv2.org/e\\xiv2-0.25.tar.gz
        #URL_MD5 258d4831b30f75a01e0234065c6c2806

        # To use current implementation from SVN

        SVN_REPOSITORY svn://dev.exiv2.org/svn/trunk

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.macports ${PREFIX_ext_exiv2} RelWithDebInfo x86_64 ${BASH_OPTIONS}

        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

else() # Linux

    ExternalProject_Add(ext_hugin
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        # To use official release tarball

        URL https://sourceforge.net/projects/hugin/files/hugin/hugin-2013.0/hugin-2013.0.0.tar.bz2/download
        URL_MD5 cc6c768df2aedc24a9a081754de05f39

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/hugin-appimage.patch

        INSTALL_DIR ${PREFIX_ext_hugin}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_hugin}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   ${Hugin_Conf}
                   ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
