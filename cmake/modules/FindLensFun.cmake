# - Try to find lensfun
# Once done this will define
#
#  LensFun_FOUND        - system has lensfun
#  LENSFUN_INCLUDE_DIRS - the lensfun include directory
#  LENSFUN_LIBRARIES    - Link these to use lensfun
#  LENSFUN_DEFINITIONS  - Compiler switches required for using lensfun
#  LENSFUN_VERSION      - library version
#
#  Copyright (c) 2008 Adrian Schroeter <adrian at suse dot de>
#  Copyright (c) 2012 Pino Toscano <pino at kde dot org>
#  Copyright (c) 2012 Ananta Palani <anantapalani at gmail dot com>
#  Copyright (c) 2012 Caulier Gilles <caulier dot gilles at gmail dot com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

IF(LENSFUN_LIBRARIES AND LENSFUN_INCLUDE_DIRS AND LENSFUN_DEFINITIONS AND LENSFUN_VERSION)

    # in cache already
    SET(LensFun_FOUND TRUE)

ELSE()

    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    IF (NOT WIN32)

        FIND_PACKAGE(PkgConfig)

        IF (PKG_CONFIG_FOUND)

            IF (LensFun_FIND_VERSION)
                SET(version_string ">=${LensFun_FIND_VERSION}")
            ENDIF()

           PKG_CHECK_MODULES(PC_LENSFUN lensfun${version_string})

        ENDIF ()

    ENDIF (NOT WIN32)

    FIND_PATH(LENSFUN_INCLUDE_DIRS lensfun.h
        HINTS ${PC_LENSFUN_INCLUDE_DIRS}
        PATH_SUFFIXES lensfun
    )

    FIND_LIBRARY(LENSFUN_LIBRARIES NAMES lensfun liblensfun
        HINTS ${PC_LENSFUN_LIBRARY_DIRS}
    )

    SET(LENSFUN_VERSION "${PC_LENSFUN_VERSION}")
    SET(LENSFUN_DEFINITIONS ${PC_LENSFUN_CFLAGS_OTHER})

    IF (LENSFUN_INCLUDE_DIRS)

        FILE(READ ${LENSFUN_INCLUDE_DIRS}/lensfun.h LENSFUN_VERSION_CONTENT)
        STRING(REGEX MATCH "#define[ \t]+LF_VERSION_MAJOR[ \t]+[0-9]*[\r\n]*.*#define[ \t]+LF_VERSION_MINOR[ \t]+[0-9]*[\r\n]*.*#define[ \t]+LF_VERSION_MICRO[ \t]+[0-9]*[\r\n]*.*#define[ \t]+LF_VERSION_BUGFIX[ \t]+[0-9]*" LENSFUN_VERSION_MATCH ${LENSFUN_VERSION_CONTENT})

        IF (LENSFUN_VERSION_MATCH)
            STRING(REGEX REPLACE "#define[ \t]+LF_VERSION_MAJOR[ \t]+([0-9]*)[\r\n]*.*#define[ \t]+LF_VERSION_MINOR[ \t]+([0-9]*)[\r\n]*.*#define[ \t]+LF_VERSION_MICRO[ \t]+([0-9]*)[\r\n]*.*#define[ \t]+LF_VERSION_BUGFIX[ \t]+([0-9]*)" "\\1.\\2.\\3.\\4" LENSFUN_VERSION ${LENSFUN_VERSION_MATCH})
        ENDIF (LENSFUN_VERSION_MATCH)

    ENDIF (LENSFUN_INCLUDE_DIRS)

    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(LensFun
                                      REQUIRED_VARS LENSFUN_INCLUDE_DIRS LENSFUN_LIBRARIES
                                      VERSION_VAR LENSFUN_VERSION
    )

    # show the variables only in the advanced view
    MARK_AS_ADVANCED(LENSFUN_INCLUDE_DIRS LENSFUN_LIBRARIES LENSFUN_DEFINITIONS LENSFUN_VERSION)

ENDIF()

