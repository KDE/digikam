# Script to build ffmpeg libraries for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(PREFIX_ext_ffmpeg "${EXTPREFIX}")

ExternalProject_Add(ext_ffmpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://ffmpeg.org/releases/ffmpeg-3.3.6.tar.xz
    URL_MD5 e7701ebfce77e44f7b4cc57dd001e7eb

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_ffmpeg}
                                             --datadir=${CMAKE_BINARY_DIR}/etc
                                             --enable-shared
                                             --enable-gpl
                                             --enable-version3
                                             --enable-nonfree
                                             --disable-doc
                                             --disable-debug
                                             --disable-ffplay
                                             --disable-ffserver
                                             --disable-outdevs
                                             --enable-runtime-cpudetect
                                             --extra-libs=-lstdc++
                                             --enable-libopencore-amrnb
                                             --enable-libopencore-amrwb
                                             --enable-libfdk-aac
                                             --enable-libmp3lame
                                             --enable-libspeex
                                             --enable-libtheora
                                             --enable-libvorbis
                                             --enable-libx264
                                             --enable-libx265
                                             --enable-libxvid
                                             --enable-libvpx
                                             --enable-libopus
                                             --enable-librtmp
                                             --enable-libfreetype
                                             --enable-libass
                                             --enable-openssl
    UPDATE_COMMAND ""
    ALWAYS 0
)
