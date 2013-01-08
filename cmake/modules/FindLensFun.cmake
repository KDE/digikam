# - Try to find lensfun
# Once done this will define
#
#  LENSFUN_FOUND        - system has lensfun
#  LENSFUN_INCLUDE_DIRS - the lensfun include directory
#  LENSFUN_LIBRARIES    - Link these to use lensfun
#  LENSFUN_DEFINITIONS  - Compiler switches required for using lensfun
#  LENSFUN_VERSION      - library version
#
#  Copyright (c) 2008 Adrian Schroeter <adrian@suse.de>
#  Copyright (c) 2012 Pino Toscano <pino@kde.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

IF (LENSFUN_LIBRARIES AND LENSFUN_INCLUDE_DIRS)

    # in cache already
    SET(LENSFUN_FOUND TRUE)

ELSE (LENSFUN_LIBRARIES AND LENSFUN_INCLUDE_DIRS)

    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    FIND_PACKAGE(PkgConfig)

    IF (PKG_CONFIG_FOUND)

        IF (LensFun_FIND_VERSION)
            SET(version_string ">=${LensFun_FIND_VERSION}")
        ENDIF()

       PKG_CHECK_MODULES(PC_LENSFUN lensfun${version_string})

    ENDIF ()

    FIND_PATH(LENSFUN_INCLUDE_DIRS lensfun.h
        HINTS ${PC_LENSFUN_INCLUDE_DIRS}
        PATH_SUFFIXES lensfun
    )

    FIND_LIBRARY(LENSFUN_LIBRARIES NAMES lensfun
        HINTS ${PC_LENSFUN_LIBRARY_DIRS}
    )

    SET(LENSFUN_VERSION "${PC_LENSFUN_VERSION}")
    SET(LENSFUN_DEFINITIONS ${PC_LENSFUN_CFLAGS_OTHER})

    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(LensFun
                                      REQUIRED_VARS LENSFUN_INCLUDE_DIRS LENSFUN_LIBRARIES
                                      VERSION_VAR LENSFUN_VERSION
    )

    # show the LENSFUN_INCLUDE_DIRS and LENSFUN_LIBRARIES variables only in the advanced view
    MARK_AS_ADVANCED(LENSFUN_INCLUDE_DIRS LENSFUN_LIBRARIES)

ENDIF (LENSFUN_LIBRARIES AND LENSFUN_INCLUDE_DIRS)

