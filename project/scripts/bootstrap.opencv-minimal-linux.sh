#!/bin/sh

# Copyright (c) 2008-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Copy this script on root folder where are source code

export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=/usr/lib/kde4
export PKG_CONFIG_PATH=/usr/lib/pkgconfig

#export VERBOSE=1

# We will work on command line using MinGW compiler
export MAKEFILES_TYPE='Unix Makefiles'

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

export OpenCVOptions='-DBUILD_EXAMPLES=OFF \
                      -DBUILD_TESTS=OFF \
                      -DBUILD_DOCS=OFF \
                      -DBUILD_PERF_TESTS=OFF \
                      -DBUILD_NEW_PYTHON_SUPPORT=OFF \
                      -DBUILD_ZLIB=OFF \
                      -DOPENCV_BUILD_3RDPARTY_LIBS=OFF \
                      -DINSTALL_C_EXAMPLES=OFF \
                      -DINSTALL_PYTHON_EXAMPLES=OFF \
                      -DBUILD_opencv_core=ON \
                      -DBUILD_opencv_imgproc=ON \
                      -DBUILD_opencv_imgcodecs=ON \
                      -DBUILD_opencv_objdetect=ON \
                      -DBUILD_opencv_calib3d=ON \
                      -DBUILD_opencv_features2d=ON \
                      -DBUILD_opencv_photo=OFF \
                      -DBUILD_opencv_java=OFF \
                      -DBUILD_opencv_java_bindings_generator=OFF \
                      -DBUILD_opencv_js=ON \
                      -DBUILD_opencv_python2=OFF \
                      -DBUILD_opencv_python3=OFF \
                      -DBUILD_opencv_python_bindings_generator=OFF \
                      -DBUILD_opencv_ml=OFF \
                      -DBUILD_opencv_shape=OFF \
                      -DBUILD_opencv_highgui=OFF \
                      -DBUILD_opencv_superres=OFF \
                      -DBUILD_opencv_stitching=OFF \
                      -DBUILD_opencv_videostab=OFF \
                      -DBUILD_opencv_videoio=OFF \
                      -DBUILD_opencv_video=OFF \
                      -DBUILD_opencv_apps=OFF \
                      -DBUILD_opencv_dnn=OFF \
                      -DBUILD_opencv_gapi=OFF \
                      -DBUILD_opencv_flann=OFF \
                      -DWITH_1394=OFF \
                      -DWITH_VTK=OFF \
                      -DWITH_DIRECTX=OFF \
                      -DWITH_DSHOW=OFF \
                      -DWITH_EIGEN=OFF \
                      -DWITH_FFMPEG=OFF \
                      -DWITH_GSTREAMER=OFF \
                      -DWITH_GTK=OFF \
                      -DWITH_IPP=OFF \
                      -DWITH_JASPER=OFF \
                      -DWITH_JPEG=OFF \
                      -DWITH_MATLAB=OFF \
                      -DWITH_OPENEXR=OFF \
                      -DWITH_OPENNI=OFF \
                      -DWITH_PNG=OFF \
                      -DWITH_PVAPI=OFF \
                      -DWITH_WIN32UI=OFF \
                      -DWITH_QT=OFF \
                      -DWITH_QUICKTIME=OFF \
                      -DWITH_QT_OPENGL=OFF \
                      -DWITH_TBB=OFF \
                      -DWITH_TIFF=OFF \
                      -DWITH_UNICAP=OFF \
                      -DWITH_V4L=OFF \
                      -DWITH_VFW=OFF \
                      -DWITH_VIDEOINPUT=OFF \
                      -DWITH_XINE=OFF \
                      -DWITH_VA_INTEL=OFF \
                      -DWITH_GPHOTO2=OFF \
                      -DWITH_PROTOBUF=OFF \
                      -DWITH_WEBP=OFF \
                      -DWITH_IMGCODEC_HDR=OFF \
                      -DWITH_IMGCODEC_SUNRASTER=OFF \
                      -DWITH_IMGCODEC_PXM=OFF \
                      -DWITH_CUDA=OFF \
                      -DWITH_CUFFT=OFF \
                      -DWITH_CUBLAS=OFF \
                      -DWITH_NVCUVID=OFF \
                      -DWITH_OPENCL=OFF \
                      -DWITH_OPENCL_SVM=OFF \
                      -DWITH_OPENCLAMDFFT=OFF \
                      -DWITH_OPENCLAMDBLAS=OFF \
                      -DCV_ENABLE_INTRINSICS=OFF \
                      -DCV_DISABLE_OPTIMIZATION=ON \
                      -DCV_TRACE=OFF'

cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Wno-dev \
      $OpenCVOptions \
      ..

exit


