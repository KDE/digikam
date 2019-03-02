# Script to build OpenCV for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(EXTPREFIX_opencv "${EXTPREFIX}" )

set(Opencv_Conf -DBUILD_EXAMPLES=OFF
                -DBUILD_TESTS=OFF
                -DBUILD_DOCS=OFF
                -DBUILD_PERF_TESTS=OFF
                -DBUILD_NEW_PYTHON_SUPPORT=OFF
                -DBUILD_ZLIB=OFF
                -DOPENCV_BUILD_3RDPARTY_LIBS=OFF
                -DINSTALL_C_EXAMPLES=OFF
                -DINSTALL_PYTHON_EXAMPLES=OFF
                -DBUILD_opencv_core=ON
                -DBUILD_opencv_imgproc=ON
                -DBUILD_opencv_imgcodecs=ON
                -DBUILD_opencv_objdetect=ON
                -DBUILD_opencv_calib3d=ON
                -DBUILD_opencv_features2d=ON
                -DBUILD_opencv_java=OFF
                -DBUILD_opencv_java_bindings_generator=OFF
                -DBUILD_opencv_js=ON
                -DBUILD_opencv_python2=OFF
                -DBUILD_opencv_python3=OFF
                -DBUILD_opencv_python_bindings_generator=OFF
                -DBUILD_opencv_ml=OFF
                -DBUILD_opencv_photo=OFF
                -DBUILD_opencv_shape=OFF
                -DBUILD_opencv_highgui=OFF
                -DBUILD_opencv_superres=OFF
                -DBUILD_opencv_stitching=OFF
                -DBUILD_opencv_videostab=OFF
                -DBUILD_opencv_videoio=OFF
                -DBUILD_opencv_video=OFF
                -DBUILD_opencv_apps=OFF
                -DBUILD_opencv_dnn=OFF
                -DBUILD_opencv_gapi=OFF
                -DBUILD_opencv_flann=OFF
                -DWITH_1394=OFF
                -DWITH_VTK=OFF
                -DWITH_DIRECTX=OFF
                -DWITH_DSHOW=OFF
                -DWITH_EIGEN=OFF
                -DWITH_FFMPEG=OFF
                -DWITH_GSTREAMER=OFF
                -DWITH_GTK=OFF
                -DWITH_IPP=OFF
                -DWITH_JASPER=OFF
                -DWITH_JPEG=OFF
                -DWITH_MATLAB=OFF
                -DWITH_OPENEXR=OFF
                -DWITH_OPENNI=OFF
                -DWITH_PNG=OFF
                -DWITH_PVAPI=OFF
                -DWITH_WIN32UI=OFF
                -DWITH_QT=OFF
                -DWITH_QUICKTIME=OFF
                -DWITH_QT_OPENGL=OFF
                -DWITH_TBB=OFF
                -DWITH_TIFF=OFF
                -DWITH_UNICAP=OFF
                -DWITH_V4L=OFF
                -DWITH_VFW=OFF
                -DWITH_VIDEOINPUT=OFF
                -DWITH_XINE=OFF
                -DWITH_VA_INTEL=OFF
                -DWITH_GPHOTO2=OFF
                -DWITH_PROTOBUF=OFF
                -DWITH_WEBP=OFF
                -DWITH_IMGCODEC_HDR=OFF
                -DWITH_IMGCODEC_SUNRASTER=OFF
                -DWITH_IMGCODEC_PXM=OFF
                -DWITH_CUDA=OFF
                -DWITH_CUFFT=OFF
                -DWITH_CUBLAS=OFF
                -DWITH_NVCUVID=OFF
                -DWITH_OPENCL=OFF
                -DWITH_OPENCL_SVM=OFF
                -DWITH_OPENCLAMDFFT=OFF
                -DWITH_OPENCLAMDBLAS=OFF
                -DCV_ENABLE_INTRINSICS=OFF
                -DCV_DISABLE_OPTIMIZATION=ON
                -DCV_TRACE=OFF
)

JoinListAsString("${Opencv_Conf}" " " BASH_OPTIONS)

if(MINGW)

    ExternalProject_Add(ext_opencv

        URL https://github.com/opencv/opencv/archive/3.4.5.tar.gz
        URL_MD5 4002be887e8cd918486684de76a4ca7f

        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN}
                   -DCMAKE_INSTALL_PREFIX=${MXE_INSTALL_PREFIX}
                   -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
                   ${GLOBAL_PROFILE}
                   ${Opencv_Conf}

        UPDATE_COMMAND ""
        ALWAYS 0
    )

    ExternalProject_Add_step(ext_opencv before_download

        # Note : under MXE, in case of curl is not compiled with https support,
        # we cannot download the tarball as under Linux/MacOS.
#        COMMAND wget -N https://github.com/opencv/opencv/archive/3.4.1.tar.gz -O ${EXTERNALS_DOWNLOAD_DIR}/3.4.1.zip
#        DEPENDERS download
    )

elseif(APPLE)

    ExternalProject_Add(ext_opencv
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL https://github.com/opencv/opencv/archive/3.4.5.tar.gz
        URL_MD5 4002be887e8cd918486684de76a4ca7f

        CONFIGURE_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bootstrap.macports <SOURCE_DIR>/. &&
                          <SOURCE_DIR>/bootstrap.macports ${EXTPREFIX_opencv} RelWithDebInfo x86_64 "${BASH_OPTIONS}"

        BUILD_COMMAND cd <SOURCE_DIR>/build && $(MAKE)

        INSTALL_COMMAND cd <SOURCE_DIR>/build && make install/fast

        BUILD_IN_SOURCE 1

        UPDATE_COMMAND ""
        ALWAYS 0
    )

else() #Linux

    ExternalProject_Add(ext_opencv
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

        URL https://github.com/opencv/opencv/archive/3.4.5.tar.gz
        URL_MD5 4002be887e8cd918486684de76a4ca7f

        INSTALL_DIR ${EXTPREFIX_opencv}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_opencv}
                   -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                   ${GLOBAL_PROFILE}
                   ${Opencv_Conf}

        UPDATE_COMMAND ""
        ALWAYS 0
    )

endif()
