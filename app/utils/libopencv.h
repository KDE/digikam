/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-16
 * @brief  Wrapper for OpenCV header files
 *
 * @author Copyright (C) 2012-2014 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef LIB_OPEN_CV_H
#define LIB_OPEN_CV_H

// Pragma directives to reduce warnings from OpenCV header files.
#if not defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#pragma clang diagnostic ignored "-Wcast-align"
#endif

// OpenCV includes

#include <opencv2/core/version.hpp>

#define OPENCV_MAKE_VERSION(major,minor,patch) (((major) << 16) | ((minor) << 8) | (patch))
#define OPENCV_VERSION                         OPENCV_MAKE_VERSION(CV_MAJOR_VERSION,CV_MINOR_VERSION,CV_SUBMINOR_VERSION)
#define OPENCV_TEST_VERSION(major,minor,patch) ( OPENCV_VERSION >= OPENCV_MAKE_VERSION(major,minor,patch) )

#if OPENCV_TEST_VERSION(2,3,0)
#   include <opencv2/opencv.hpp>
#   include <opencv2/legacy/compat.hpp>
#   include <opencv/cvaux.h>
#else
#   include <opencv/cv.h>
#   include <opencv/cvaux.h>
#   include <opencv/cxcore.h>
#   include <opencv/highgui.h>
#endif

// Restore warnings
#if not defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // LIB_OPEN_CV_H
