# - Try to find the QtAV library
#
# Once done this will define
#
#  QTAV_FOUND        - system has libqtav
#  QTAV_INCLUDE_DIRS - the libqtav include directory
#  QTAV_LIBRARIES    - Link these to use libqtav
#
# Copyright (c) 2016-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(Qt5
             QUIET REQUIRED NO_MODULE COMPONENTS
             Core
)

find_path(QTAV_CORE_INCLUDE_DIR
          NAMES QtAV.h
          HINTS ${_qt5_install_prefix}
          PATH_SUFFIXES QtAV
)

find_path(QTAV_WIDGETS_INCLUDE_DIR
          NAMES QtAVWidgets.h
          HINTS ${_qt5_install_prefix}
          PATH_SUFFIXES QtAVWidgets
)

find_library(QTAV_CORE_LIBRARY NAMES QtAV QtAV1 libQtAV libQtAV1)

find_library(QTAV_WIDGETS_LIBRARY NAMES QtAVWidgets QtAVWidgets1 libQtAVWidgets libQtAVWidgets1)

set(QTAV_INCLUDE_DIRS "${QTAV_CORE_INCLUDE_DIR} ${QTAV_WIDGETS_INCLUDE_DIR}")
set(QTAV_LIBRARIES     ${QTAV_CORE_LIBRARY} ${QTAV_WIDGETS_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QtAV REQUIRED_VARS QTAV_LIBRARIES QTAV_INCLUDE_DIRS)

mark_as_advanced(QTAV_INCLUDE_DIRS QTAV_LIBRARIES)

message(STATUS "QtAV_FOUND       = ${QTAV_FOUND}")
message(STATUS "QtAV_INCLUDE_DIR = ${QTAV_INCLUDE_DIRS}")
message(STATUS "QtAV_LIBRARIES   = ${QTAV_LIBRARIES}")
