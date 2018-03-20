# - Try to find the QtAV library
#
# Once done this will define
#
#  QtAV_FOUND          - The system has libqtav
#  QTAV_INCLUDE_DIRS   - The libqtav include directory
#  QTAV_LIBRARIES      - Link these to use libqtav
#  QTAV_MAJOR_VERSION  - The major value of QtAV version ID defined in QtAV/version.h as "1".
#  QTAV_MINOR_VERSION  - The minor value of QtAV version ID defined in QtAV/version.h as "12".
#  QTAV_PATCH_VERSION  - The patch value of QtAV version ID defined in QtAV/version.h as "0".
#  QTAV_VERSION_STRING - Version string e.g. "1.12.0"
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
              HINTS ${Qt5Core_INCLUDE_DIRS}
                    ${_qt5_install_prefix}                                           # For MXE
                    ${_qt5_install_prefix}/../qt5/include                            # For Mageia
                    ${_qt5_install_prefix}/../../include/qt5                         # For Suse
                    ${_qt5_install_prefix}/../../../include/${CMAKE_LIBRARY_ARCHITECTURE}/qt5     # For Debian
                    ${_qt5_install_prefix}/../../include/qt                          # For Arch
              PATH_SUFFIXES QtAV
    )

    find_path(QTAV_WIDGETS_INCLUDE_DIR
              NAMES QtAVWidgets.h
              HINTS ${Qt5Core_INCLUDE_DIRS}
                    ${_qt5_install_prefix}                                           # For MXE
                    ${_qt5_install_prefix}/../qt5/include                            # For Mageia
                    ${_qt5_install_prefix}/../../include/qt5                         # For Suse
                    ${_qt5_install_prefix}/../../../include/${CMAKE_LIBRARY_ARCHITECTURE}/qt5     # For Debian
                    ${_qt5_install_prefix}/../../include/qt                          # For Arch
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

    find_path(QTAV_CORE_INCLUDE_DIR
              NAMES QtAV/QtAV.h
              HINTS ${_qt5_install_prefix}
                    ${_qt5_install_prefix}/../../include
              PATH_SUFFIXES QtAV
    )

    find_path(QTAV_WIDGETS_INCLUDE_DIR
              NAMES QtAVWidgets/QtAVWidgets.h
              HINTS ${_qt5_install_prefix}
                    ${_qt5_install_prefix}/../../include
              PATH_SUFFIXES QtAVWidgets
    )

    if (QTAV_CORE_INCLUDE_DIR AND QTAV_WIDGETS_INCLUDE_DIR)
        set(QTAV_INCLUDE_DIRS "${QTAV_CORE_INCLUDE_DIR};${QTAV_WIDGETS_INCLUDE_DIR}")
        set(QTAV_LIBRARIES    "${_qt5_install_prefix}/../QtAV.framework/QtAV;${_qt5_install_prefix}/../QtAVWidgets.framework/QtAVWidgets")
    else()
        set(QTAV_INCLUDE_DIRS "${_qt5_install_prefix}/../../include/QtAV;${_qt5_install_prefix}/../../include/QtAVWidgets")
        set(QTAV_LIBRARIES    "${_qt5_install_prefix}/../QtAV.framework/QtAV;${_qt5_install_prefix}/../QtAVWidgets.framework/QtAVWidgets")
    endif()

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QtAV REQUIRED_VARS QTAV_LIBRARIES QTAV_INCLUDE_DIRS)

if(QtAV_FOUND)

    if (NOT APPLE)
        file(READ ${QTAV_CORE_INCLUDE_DIR}/version.h QTAV_VERSION_CONTENT)
    else()
        file(READ ${QTAV_CORE_INCLUDE_DIR}/QtAV/version.h QTAV_VERSION_CONTENT)
    endif()

    string(REGEX MATCH "#define QTAV_MAJOR ([0-9]+)" QTAV_MAJOR_MATCH ${QTAV_VERSION_CONTENT})
    string(REPLACE "#define QTAV_MAJOR " "" QTAV_MAJOR_VERSION ${QTAV_MAJOR_MATCH})
    string(REGEX MATCH "#define QTAV_MINOR ([0-9]+)" QTAV_MINOR_MATCH ${QTAV_VERSION_CONTENT})
    string(REPLACE "#define QTAV_MINOR " "" QTAV_MINOR_VERSION ${QTAV_MINOR_MATCH})
    string(REGEX MATCH "#define QTAV_PATCH ([0-9]+)" QTAV_PATCH_MATCH ${QTAV_VERSION_CONTENT})
    string(REPLACE "#define QTAV_PATCH " "" QTAV_PATCH_VERSION ${QTAV_PATCH_MATCH})

    if(NOT QTAV_MAJOR_VERSION STREQUAL "" AND
       NOT QTAV_MINOR_VERSION STREQUAL "" AND
       NOT QTAV_PATCH_VERSION STREQUAL "")

            set(QTAV_VERSION_STRING "${QTAV_MAJOR_VERSION}.${QTAV_MINOR_VERSION}.${QTAV_PATCH_VERSION}")

            message(STATUS "Found QtAV version ${QTAV_VERSION_STRING}")

    else()

        message(STATUS "Found QtAV but failed to find version ${QTAV_LIBRARIES}")
        set(QTAV_MAJOR_VERSION NOTFOUND)
        set(QTAV_MINOR_VERSION NOTFOUND)
        set(QTAV_PATCH_VERSION NOTFOUND)
        set(QTAV_VERSION_STRING NOTFOUND)

    endif()

endif()

mark_as_advanced(QTAV_INCLUDE_DIRS QTAV_LIBRARIES QTAV_VERSION_STRING
                 QTAV_MAJOR_VERSION QTAV_MINOR_VERSION QTAV_PATCH_VERSION)

message(STATUS "QtAV_FOUND       = ${QtAV_FOUND}")
message(STATUS "QtAV_INCLUDE_DIR = ${QTAV_INCLUDE_DIRS}")
message(STATUS "QtAV_LIBRARIES   = ${QTAV_LIBRARIES}")
message(STATUS "QtAV_VERSION     = ${QTAV_VERSION_STRING}")
