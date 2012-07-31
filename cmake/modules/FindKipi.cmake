# Module that tries to find the Kipi library
#
# If you have put a local version of libkipi into your source tree,
# set KIPI_LOCAL_DIR to the relative path to the local directory.
#
# Once done this will define
#
#  KIPI_FOUND       - system has libkipi
#  KIPI_INCLUDE_DIR - the libkipi include directory
#  KIPI_LIBRARIES   - Link these to use libkipi
#  KIPI_DEFINITIONS - Compiler switches required for using libkipi
#  KIPI_VERSION     - The version of the Kipi library
#

# Copyright (c) 2012, Victor Dodon <dodonvictor at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF(KIPI_INCLUDE_DIR AND KIPI_LIBRARIES AND KIPI_DEFINITIONS AND KIPI_VERSION)

  MESSAGE(STATUS "Found kipi library in cache ${KIPI_LIBRARIES}")
  # Already in cache
  SET(KIPI_FOUND TRUE)

ELSE(KIPI_INCLUDE_DIR AND KIPI_LIBRARIES AND KIPI_DEFINITIONS AND KIPI_VERSION)

  MESSAGE(STATUS "Check Kipi library in local sub-folder...")

  IF(KIPI_LOCAL_DIR)
    SET(KIPI_LOCAL_FOUND TRUE)
  ELSE(KIPI_LOCAL_DIR)
    FIND_FILE(KIPI_LOCAL_FOUND libkipi/plugin.h ${CMAKE_SOURCE_DIR}/libkipi
      ${CMAKE_SOURCE_DIR}/libs/libkipi NO_DEFAULT_PATH)

    IF (KIPI_LOCAL_FOUND)
      FIND_FILE(KIPI_LOCAL_FOUND_IN_LIBS libkipi/plugin.h
        ${CMAKE_SOURCE_DIR}/libs/libkipi NO_DEFAULT_PATH)
      IF(KIPI_LOCAL_FOUND_IN_LIBS)
        SET(KIPI_LOCAL_DIR libs/libkipi)
      ELSE(KIPI_LOCAL_FOUND_IN_LIBS)
        SET(KIPI_LOCAL_DIR libkipi)
      ENDIF(KIPI_LOCAL_FOUND_IN_LIBS)
    ENDIF(KIPI_LOCAL_FOUND)
  ENDIF(KIPI_LOCAL_DIR)

  IF(KIPI_LOCAL_FOUND)

    SET(KIPI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${KIPI_LOCAL_DIR}
      ${CMAKE_BINARY_DIR}/${KIPI_LOCAL_DIR})
    SET(KIPI_DEFINITIONS "-I${CMAKE_SOURCE_DIR}/${KIPI_LOCAL_DIR}"
      "-I${CMAKE_BINARY_DIR}/${KIPI_LOCAL_DIR}")
    SET(KIPI_LIBRARIES kipi)
    MESSAGE(STATUS "Found Kipi library in local sub-folder: ${CMAKE_SOURCE_DIR}/${KIPI_LOCAL_DIR}")

    SET(KIPI_FOUND TRUE)

    SET(KIPI_CMAKE_FILE ${CMAKE_SOURCE_DIR}/${KIPI_LOCAL_DIR}/CMakeLists.txt)
    FILE(READ ${KIPI_CMAKE_FILE} KIPI_CMAKE_FILE_CONTENT)
    STRING(REGEX REPLACE ".*[\n|^]SET[ \t]*[(]KIPI_LIB_MAJOR_VERSION \"([0-9]+)\"[)][ \t]*[\n|$].*"
      "\\1" KIPI_MAJOR_VERSION "${KIPI_CMAKE_FILE_CONTENT}")
    STRING(REGEX REPLACE ".*[\n|^]SET[ \t]*[(]KIPI_LIB_MINOR_VERSION \"([0-9]+)\"[)][ \t]*[\n|$].*"
      "\\1" KIPI_MINOR_VERSION "${KIPI_CMAKE_FILE_CONTENT}")
    STRING(REGEX REPLACE ".*[\n|^]SET[ \t]*[(]KIPI_LIB_PATCH_VERSION \"([0-9]+)\"[)][ \t]*[\n|$].*"
      "\\1" KIPI_PATCH_VERSION "${KIPI_CMAKE_FILE_CONTENT}")
    STRING(REGEX REPLACE ".*[\n|^]SET[ \t]*[(]KIPI_LIB_SUFFIX_VERSION \"([^\"]*)\"[)][ \t]*[\n|$].*"
      "\\1" KIPI_SUFFIX_VERSION "${KIPI_CMAKE_FILE_CONTENT}")
    SET(KIPI_VERSION "${KIPI_MAJOR_VERSION}.${KIPI_MINOR_VERSION}.${KIPI_PATCH_VERSION}${KIPI_SUFFIX_VERSION}")

  ELSE(KIPI_LOCAL_FOUND)

    IF(NOT WIN32)
      INCLUDE(FindPkgConfig)
      PKG_CHECK_MODULES(KIPI libkipi>=2.0.0)
    ENDIF(NOT WIN32)

    FIND_LIBRARY(KIPI_LIBRARIES
      NAMES
      libkipi
      PATHS
      ${KIPI_LIBRARY_DIRS}
      ${LIB_INSTALL_DIR}
      ${KDE4_LIB_DIR}
    )

    FIND_PATH(KIPI_INCLUDE_DIR
      NAMES
      libkipi/version.h
      PATHS
      ${KIPI_INCLUDE_DIRS}
      ${INCLUDE_INSTALL_DIR}
      ${KDE4_INCLUDE_DIR}
    )

    SET(KIPI_DEFINITIONS ${KIPI_CFLAGS})

    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(KIPI DEFAULT_MSG KIPI_LIBRARIES KIPI_INCLUDE_DIR)

  ENDIF(KIPI_LOCAL_FOUND)

  MARK_AS_ADVANCED(KIPI_INCLUDE_DIR KIPI_LIBRARIES KIPI_DEFINITIONS KIPI_VERSION)

ENDIF(KIPI_INCLUDE_DIR AND KIPI_LIBRARIES AND KIPI_DEFINITIONS AND KIPI_VERSION)
