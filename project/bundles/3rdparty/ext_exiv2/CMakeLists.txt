# Script to build Exiv2 for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(PREFIX_ext_exiv2 "${EXTPREFIX}")

set(Exiv2_Conf -DBUILD_SHARED_LIBS=ON
               -DEXIV2_ENABLE_XMP=ON
               -DEXIV2_ENABLE_EXTERNAL_XMP=OFF
               -DEXIV2_ENABLE_PNG=ON
               -DEXIV2_ENABLE_NLS=ON
               -DEXIV2_ENABLE_PRINTUCS2=ON
               -DEXIV2_ENABLE_LENSDATA=ON
               -DEXIV2_ENABLE_VIDEO=OFF
               -DEXIV2_ENABLE_WEBREADY=OFF
               -DEXIV2_ENABLE_DYNAMIC_RUNTIME=OFF
               -DEXIV2_ENABLE_CURL=OFF
               -DEXIV2_ENABLE_SSH=OFF
               -DEXIV2_BUILD_SAMPLES=OFF
               -DEXIV2_BUILD_PO=OFF
               -DEXIV2_BUILD_EXIV2_COMMAND=OFF
               -DEXIV2_BUILD_UNIT_TESTS=OFF
               -DEXIV2_BUILD_DOC=OFF
               -DEXIV2_TEAM_EXTRA_WARNINGS=OFF
               -DEXIV2_TEAM_WARNINGS_AS_ERRORS=OFF
               -DEXIV2_TEAM_USE_SANITIZERS=OFF
               -DEXIV2_TEAM_PACKAGING=OFF
)

if(MINGW)

    set(Exiv2_Conf ${Exiv2_Conf}
               -DEXIV2_ENABLE_WIN_UNICODE=ON
    )

endif()

JoinListAsString("${Exiv2_Conf}" " " BASH_OPTIONS)

if(MINGW)

    ExternalProject_Add(ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        # To use official release tarball
        URL https://github.com/Exiv2/exiv2/archive/v0.27.2.tar.gz
        URL_MD5 15780152b1dfb5a0d2c0cdfe5f5a1588

        # To use current implementation from Git with tag 0.27 rc1

        #GIT_REPOSITORY https://github.com/Exiv2/exiv2.git
        #GIT_TAG v0.27-RC1

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.mxe <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.mxe ${MXE_BUILDROOT} RelWithDebInfo ${BASH_OPTIONS}

        BUILD_COMMAND cd <SOURCE_DIR>/build.mxe && $(MAKE) -j

        INSTALL_COMMAND cd <SOURCE_DIR>/build.mxe && $(MAKE) install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )


elseif(APPLE)

    ExternalProject_Add(ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        # To use official release tarball

        URL https://github.com/Exiv2/exiv2/archive/v0.27.2.tar.gz
        URL_MD5 15780152b1dfb5a0d2c0cdfe5f5a1588

        # To use current implementation from Git

        #GIT_REPOSITORY https://github.com/Exiv2/exiv2.git

        CONFIGURE_COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.macports ${PREFIX_ext_exiv2} RelWithDebInfo x86_64 ${BASH_OPTIONS}

        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR>/build && $(MAKE) install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

else() # Linux

    ExternalProject_Add(ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        # To use official release tarball

        URL https://github.com/Exiv2/exiv2/archive/v0.27.2.tar.gz
        URL_MD5 15780152b1dfb5a0d2c0cdfe5f5a1588

        # To use current implementation from Git

        #GIT_REPOSITORY https://github.com/Exiv2/exiv2.git

        CONFIGURE_COMMAND cd <SOURCE_DIR> &&
                   $(CMAKE_COMMAND) . -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   -DICONV_INCLUDE_DIR=${PREFIX_ext_exiv2}/include
                   ${Exiv2_Conf}
                   ${GLOBAL_PROFILE}

        BUILD_COMMAND cd <SOURCE_DIR> && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR> && $(MAKE) install/fast

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
