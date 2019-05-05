# Script to build libheif for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(PREFIX_ext_heif "${EXTPREFIX}")

if(MINGW)

    ExternalProject_Add(de265
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/strukturag/libde265/archive/v1.0.3.tar.gz
        URL_MD5 189b4deb376ff879f8cbb2b7ffc24f97

        INSTALL_DIR ${EXTPREFIX}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   ${GLOBAL_PROFILE}

        INSTALL_COMMAND mkdir -p ${EXTPREFIX}/include/libde265/ &&
                        cp -fv <BINARY_DIR>/libde265/de265-version.h ${EXTPREFIX}/include/libde265/ &&
                        cp -fv <SOURCE_DIR>/libde265/de265.h ${EXTPREFIX}/include/libde265/ &&
                        cp -fv <BINARY_DIR>/libde265/libde265.so ${EXTPREFIX}/lib/ &&
                        cp -fv ${CMAKE_CURRENT_SOURCE_DIR}/libde265.pc ${EXTPREFIX}/lib/pkgconfig/

        UPDATE_COMMAND ""
        ALWAYS 0
    )

    ExternalProject_Add(ext_heif
        DEPENDS de265

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/strukturag/libheif/archive/v1.4.0.tar.gz
        URL_MD5 05dea500c105289c74270791cb5f8942

        INSTALL_DIR ${PREFIX_ext_heif}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_heif}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   ${GLOBAL_PROFILE}

        INSTALL_COMMAND mkdir -p ${PREFIX_ext_heif}/include/libheif/ &&
                        cp -fv <SOURCE_DIR>/libheif/heif_version.h ${PREFIX_ext_heif}/include/libheif/ &&
                        cp -fv <SOURCE_DIR>/libheif/heif.h ${PREFIX_ext_heif}/include/libheif/ &&
                        cp -fv <SOURCE_DIR>/libheif/heif_cxx.h ${PREFIX_ext_heif}/include/libheif/ &&
                        cp -fv <SOURCE_DIR>/libheif/heif_plugin.h ${PREFIX_ext_heif}/include/libheif/ &&
                        cp -fv <BINARY_DIR>/libheif/libheif.so ${PREFIX_ext_heif}/lib/ &&
                        cp -fv ${CMAKE_CURRENT_SOURCE_DIR}/libheif.pc ${PREFIX_ext_heif}/lib/pkgconfig/

        UPDATE_COMMAND ""
        ALWAYS 0
    )


elseif(APPLE)

    # TODO

else() # Linux

    ExternalProject_Add(de265
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/strukturag/libde265/archive/v1.0.3.tar.gz
        URL_MD5 189b4deb376ff879f8cbb2b7ffc24f97

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/de265-sse-centos6.patch

        INSTALL_DIR ${EXTPREFIX}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   ${GLOBAL_PROFILE}

        INSTALL_COMMAND mkdir -p /usr/include/libde265/ &&
                        cp -fv <BINARY_DIR>/libde265/de265-version.h /usr/include/libde265/ &&
                        cp -fv <SOURCE_DIR>/libde265/de265.h /usr/include/libde265/ &&
                        cp -fv <BINARY_DIR>/libde265/libde265.so /usr/lib/ &&
                        cp -fv ${CMAKE_CURRENT_SOURCE_DIR}/libde265.pc /usr/lib/pkgconfig/

        UPDATE_COMMAND ""
        ALWAYS 0
    )

    ExternalProject_Add(ext_heif
        DEPENDS de265

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/strukturag/libheif/archive/v1.4.0.tar.gz
        URL_MD5 05dea500c105289c74270791cb5f8942

        INSTALL_DIR ${PREFIX_ext_heif}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_heif}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   ${GLOBAL_PROFILE}

        INSTALL_COMMAND mkdir -p /usr/include/libheif/ &&
                        cp -fv <SOURCE_DIR>/libheif/heif_version.h /usr/include/libheif/ &&
                        cp -fv <SOURCE_DIR>/libheif/heif.h /usr/include/libheif/ &&
                        cp -fv <SOURCE_DIR>/libheif/heif_cxx.h /usr/include/libheif/ &&
                        cp -fv <SOURCE_DIR>/libheif/heif_plugin.h /usr/include/libheif/ &&
                        cp -fv <BINARY_DIR>/libheif/libheif.so /usr/lib/ &&
                        cp -fv ${CMAKE_CURRENT_SOURCE_DIR}/libheif.pc /usr/lib/pkgconfig/

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
