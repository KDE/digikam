# Cmake macro to detect gphoto2 libraries
#
# This module defines
#  Gphoto2_FOUND          - True if libgphoto2 is detected.
#  GPHOTO2_INCLUDE_DIRS   - Path to libgphoto2 header files.
#  GPHOTO2_LIBRARIES      - Libraries to link against to use libgphoto2.
#  GPHOTO2_VERSION_STRING - e.g. "2.4.14"
#  GPHOTO2_VERSION_MAJOR  - e.g. "2"
#  GPHOTO2_VERSION_MINOR  - e.g. "4"
#  GPHOTO2_VERSION_PATCH  - e.g. "14"
#
# Copyright (c) 2006-2007 Laurent Montel <montel@kde.org>
# Copyright (c) 2011-2016 Gilles Caulier <caulier.gilles@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

set(GPHOTO2_FIND_REQUIRED ${Gphoto2_FIND_REQUIRED})

find_path(GPHOTO2_INCLUDE_DIRS gphoto2/gphoto2.h)
mark_as_advanced(GPHOTO2_INCLUDE_DIRS)

set(GPHOTO2_NAMES      ${GPHOTO2_NAMES}      gphoto2      libgphoto2)
set(GPHOTO2_PORT_NAMES ${GPHOTO2_PORT_NAMES} gphoto2_port libgphoto2_port)

find_library(GPHOTO2_LIBRARY      NAMES ${GPHOTO2_NAMES})
find_library(GPHOTO2_PORT_LIBRARY NAMES ${GPHOTO2_PORT_NAMES})

mark_as_advanced(GPHOTO2_LIBRARY)
mark_as_advanced(GPHOTO2_PORT_LIBRARY)

# Detect libgphoto2 version

find_package(PkgConfig)
pkg_check_modules(PC_GPHOTO2 QUIET libgphoto2)

if(PC_GPHOTO2_FOUND)

    set(GPHOTO2_VERSION_STRING "${PC_GPHOTO2_VERSION}")

endif()

# handle the QUIETLY and REQUIRED arguments and set Gphoto2_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gphoto2 DEFAULT_MSG
                                  GPHOTO2_LIBRARY
                                  GPHOTO2_INCLUDE_DIRS)

if(Gphoto2_FOUND)

    set(GPHOTO2_INCLUDE_DIRS ${GPHOTO2_INCLUDE_DIRS}/gphoto2)
    set(GPHOTO2_LIBRARIES    ${GPHOTO2_LIBRARY} ${GPHOTO2_PORT_LIBRARY})

    # See bug #268267: digiKam need to be linked to libusb to prevent crash
    # at gphoto2 init if opencv is linked with libdc1394.
    #
    # libgphoto2 dynamically loads and unloads usb library
    # without calling any cleanup functions (since they are absent from libusb-0.1).
    # This leaves usb event handling threads running with invalid callback and return addresses,
    # which causes a crash after any usb event is generated.
    # libusb1 backend does correctly call exit function, but ATM it crashes anyway.
    # Workaround is to link against libusb so that it wouldn't get unloaded.

    find_library(USB1_LIBRARY NAMES usb-1.0 libusb-1.0)
    mark_as_advanced(USB1_LIBRARY)

    if(USB1_LIBRARY)

        set(GPHOTO2_LIBRARIES ${GPHOTO2_LIBRARIES} ${USB1_LIBRARY})

    endif()

endif()

message(STATUS "libgphoto2 found    : ${Gphoto2_FOUND}")
message(STATUS "libgphoto2 version  : ${GPHOTO2_VERSION_STRING}")
message(STATUS "libgphoto2 includes : ${GPHOTO2_INCLUDE_DIRS}")
message(STATUS "libgphoto2 libraries: ${GPHOTO2_LIBRARIES}")
