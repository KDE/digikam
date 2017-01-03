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

Message(STATUS "QtAV search path: ${_qt5_install_prefix}")

if (NOT APPLE)
    find_path(QTAV_CORE_INCLUDE_DIR
              NAMES QtAV.h
              HINTS ${_qt5_install_prefix}                                           # For MXE
                    ${_qt5_install_prefix}/../qt5/include                            # For Mageia
                    ${_qt5_install_prefix}/../../include/qt5                         # For Suse
                    ${_qt5_install_prefix}/../../../include/x86_64-linux-gnu/qt5     # For Debian
              PATH_SUFFIXES QtAV
    )

    find_path(QTAV_WIDGETS_INCLUDE_DIR
              NAMES QtAVWidgets.h
              HINTS ${_qt5_install_prefix}                                           # For MXE
                    ${_qt5_install_prefix}/../qt5/include                            # For Mageia
                    ${_qt5_install_prefix}/../../include/qt5                         # For Suse
                    ${_qt5_install_prefix}/../../../include/x86_64-linux-gnu/qt5     # For Debian
              PATH_SUFFIXES QtAVWidgets
    )

    find_library(QTAV_CORE_LIBRARY
                 NAMES QtAV
                       QtAV1
                       libQtAV
                       libQtAV1
                 HINTS ${_qt5_install_prefix}/../
    )

    find_library(QTAV_WIDGETS_LIBRARY
                 NAMES QtAVWidgets
                       QtAVWidgets1
                       libQtAVWidgets
                       libQtAVWidgets1
                 HINTS ${_qt5_install_prefix}/../
    )

    set(QTAV_INCLUDE_DIRS "${QTAV_CORE_INCLUDE_DIR} ${QTAV_WIDGETS_INCLUDE_DIR}")
    set(QTAV_LIBRARIES     ${QTAV_CORE_LIBRARY} ${QTAV_WIDGETS_LIBRARY})

else()

    set(QTAV_INCLUDE_DIRS ${_qt5_install_prefix}/../../include/QtAV ${_qt5_install_prefix}/../../include/QtAVWidgets)
    set(QTAV_LIBRARIES    "${_qt5_install_prefix}/../QtAV.framework/QtAV;${_qt5_install_prefix}/../QtAVWidgets.framework/QtAVWidgets")

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QtAV REQUIRED_VARS QTAV_LIBRARIES QTAV_INCLUDE_DIRS)

mark_as_advanced(QTAV_INCLUDE_DIRS QTAV_LIBRARIES)

message(STATUS "QtAV_FOUND       = ${QTAV_FOUND}")
message(STATUS "QtAV_INCLUDE_DIR = ${QTAV_INCLUDE_DIRS}")
message(STATUS "QtAV_LIBRARIES   = ${QTAV_LIBRARIES}")
