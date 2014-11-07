# - Try to find libpgf
# Once done this will define
#
#  PGF_FOUND            - system has libgf.
#  PGF_INCLUDE_DIRS     - the libpgf include directory.
#  PGF_LIBRARIES        - Link these to use libpgf.
#  PGF_LDFLAGS          - Linking flags.
#  PGF_CFLAGS           - Compiler flags.
#  PGF_CODEC_VERSION_ID - PGF codec version ID.
#
# Copyright (c) 2011-2014, Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF("${PGF_FIND_VERSION}" STREQUAL "")
    SET(PGF_FIND_VERSION "6.12.24")
    MESSAGE(STATUS "No PGF library version required. Check default version : ${PGF_FIND_VERSION}")
ELSE()
    MESSAGE(STATUS "PGF library version required : ${PGF_FIND_VERSION}")
ENDIF()

FIND_PACKAGE(PkgConfig)

IF(PKG_CONFIG_FOUND)
    PKG_CHECK_MODULES(PC_PGF libpgf>=${PGF_FIND_VERSION})
ENDIF()

FIND_PATH(PGF_INCLUDE_DIRS PGFtypes.h
        HINTS
        ${PC_PGF_INCLUDEDIR}
        ${PC_PGF_INCLUDE_DIRS}
        PATH_SUFFIXES libpgf
        )

FIND_LIBRARY(PGF_LIBRARIES NAMES pgf libpgf
            HINTS
            ${PC_PGF_LIBDIR}
            ${PC_PGF_LIBRARY_DIRS}
            )

IF(PGF_INCLUDE_DIRS)

    FILE(READ ${PGF_INCLUDE_DIRS}/PGFtypes.h PGF_VERSION_CONTENT)
    STRING(REGEX MATCH "#define PGFCodecVersionID[ ]*0x[0-9]*"  PGF_VERSION_CODEC_MATCH  ${PGF_VERSION_CONTENT})
    STRING(REGEX MATCH "#define PGFCodecVersion[\t]*\"[0-9]*.[0-9]*.[0-9]*\"" PGF_VERSION_STRING_MATCH ${PGF_VERSION_CONTENT})

    IF(PGF_VERSION_CODEC_MATCH)
        STRING(REGEX REPLACE "#define PGFCodecVersionID[ ]*0x([0-9]*)" "\\1" PGF_CODEC_VERSION_ID ${PGF_VERSION_CODEC_MATCH})
    ELSE()
        IF (NOT PGF_FIND_QUIETLY)
            MESSAGE(STATUS "Failed to get codec version information from ${PGF_INCLUDE_DIRS}/PGFtypes.h")
        ENDIF()
    ENDIF()

    IF(PGF_VERSION_STRING_MATCH)
        STRING(REGEX REPLACE "#define PGFCodecVersion[\t]*\"([0-9]*.[0-9]*.[0-9]*)\"" "\\1" PGF_VERSION ${PGF_VERSION_STRING_MATCH})
    ELSE()
        IF (NOT PGF_FIND_QUIETLY)
            MESSAGE(STATUS "Failed to get string version information from ${PGF_INCLUDE_DIRS}/PGFtypes.h")
        ENDIF()
    ENDIF()

ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libpgf
                                  REQUIRED_VARS PGF_LIBRARIES PGF_INCLUDE_DIRS
                                  VERSION_VAR PGF_VERSION
                                 )

MESSAGE(STATUS "PGF_CODEC_VERSION_ID = ${PGF_CODEC_VERSION_ID}")
MESSAGE(STATUS "PGF_VERSION          = ${PGF_VERSION}")
MESSAGE(STATUS "PGF_INCLUDE_DIRS     = ${PGF_INCLUDE_DIRS}")
MESSAGE(STATUS "PGF_LIBRARIES        = ${PGF_LIBRARIES}")
MESSAGE(STATUS "PGF_LDFLAGS          = ${PGF_LDFLAGS}")
MESSAGE(STATUS "PGF_CFLAGS           = ${PGF_CFLAGS}")

SET(PGF_FOUND FALSE)

IF(PGF_VERSION)
    IF(${PGF_VERSION} VERSION_LESS ${PGF_FIND_VERSION})
        MESSAGE(WARNING "PGF version is too old.")
    ELSE()
        SET(PGF_FOUND TRUE)
    ENDIF()

    IF(PGF_FOUND)
        MARK_AS_ADVANCED(PGF_CODEC_VERSION_ID
                         PGF_INCLUDE_DIRS
                         PGF_LIBRARIES
                         PGF_LDFLAGS
                         PGF_CFLAGS
                        )
    ENDIF()
ENDIF()
