# Cmake macro to detect gphoto2 libraries
#
#  GPHOTO2_FOUND - system has the GPHOTO2 library
#  GPHOTO2_INCLUDE_DIR - the GPHOTO2 include directory
#  GPHOTO2_LIBRARIES - The libraries needed to use GPHOTO2
#
# Copyright (c) 2006, 2007 Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(GPHOTO2_LIBRARIES AND GPHOTO2_INCLUDE_DIR)

    # in cache already
    set(GPHOTO2_FOUND TRUE)

else()

    find_package(PkgConfig)
    pkg_check_modules(PC_GPHOTO2 QUIET gphoto2)

    find_path(GPHOTO2_TOP_INCLUDE_DIR gphoto2/gphoto2.h
      HINTS ${PC_GPHOTO2_INCLUDEDIR})

    set(GPHOTO2_INCLUDE_DIRS ${GPHOTO2_TOP_INCLUDE_DIR}/gphoto2)

    find_library(GPHOTO2_LIBRARY NAMES gphoto2
      HINTS ${PC_GPHOTO2_LIBDIR} ${PC_GPHOTO2_LIBRARY_DIRS})

    find_library(GPHOTO2_PORT_LIBRARY NAMES gphoto2_port
      HINTS ${PC_GPHOTO2_LIBDIR} ${PC_GPHOTO2_LIBRARY_DIRS})

    set(GPHOTO2_LIBRARIES ${GPHOTO2_LIBRARY})
    list(APPEND GPHOTO2_LIBRARIES ${GPHOTO2_PORT_LIBRARY})
    set(GPHOTO2_VERSION "${PC_GPHOTO2_VERSION}")

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(gphoto2 DEFAULT_MSG GPHOTO2_LIBRARIES GPHOTO2_INCLUDE_DIRS)

    if(GPHOTO2_LIBRARIES AND GPHOTO2_INCLUDE_DIRS)

        set(GPHOTO2_FOUND TRUE)

    else()

        set(GPHOTO2_FOUND FALSE)

    endif()

endif()

mark_as_advanced(GPHOTO2_LIBRARIES GPHOTO2_INCLUDE_DIRS)
