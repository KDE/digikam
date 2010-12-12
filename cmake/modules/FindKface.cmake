# - Try to find the KFace library
#
# If you have put a local version of libkface into your source tree,
# set KFACE_LOCAL_DIR to the relative path to the local directory.
#
# Once done this will define
#
#  KFACE_FOUND - system has libkface
#  KFACE_INCLUDE_DIR - the libkface include directory
#  KFACE_LIBRARIES - Link these to use libkface
#  KFACE_DEFINITIONS - Compiler switches required for using libkface
#

# Copyright (c) 2010, Gilles Caulier, <caulier.gilles@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (KFACE_INCLUDE_DIR AND KFACE_LIBRARIES AND KFACE_DEFINITIONS)

  MESSAGE(STATUS "Found Kface library in cache: ${KFACE_LIBRARIES}")

  # in cache already
  SET(KFACE_FOUND TRUE)

ELSE (KFACE_INCLUDE_DIR AND KFACE_LIBRARIES AND KFACE_DEFINITIONS)

  MESSAGE(STATUS "Check Kface library in local sub-folder...")

  # Check if library is not in local sub-folder

  IF (KFACE_LOCAL_DIR)
    SET(KFACE_LOCAL_FOUND TRUE)
  ELSE (KFACE_LOCAL_DIR)
    FIND_FILE(KFACE_LOCAL_FOUND libkface/version.h.cmake ${CMAKE_SOURCE_DIR}/libkface ${CMAKE_SOURCE_DIR}/libs/libkface NO_DEFAULT_PATH)

    IF (KFACE_LOCAL_FOUND)
      FIND_FILE(KFACE_LOCAL_FOUND_IN_LIBS libkface/version.h.cmake ${CMAKE_SOURCE_DIR}/libs/libkface NO_DEFAULT_PATH)
      IF (KFACE_LOCAL_FOUND_IN_LIBS)
        SET(KFACE_LOCAL_DIR libs/libkface)
      ELSE (KFACE_LOCAL_FOUND_IN_LIBS)
        SET(KFACE_LOCAL_DIR libkface)
      ENDIF (KFACE_LOCAL_FOUND_IN_LIBS)
    ENDIF (KFACE_LOCAL_FOUND)
  ENDIF (KFACE_LOCAL_DIR)

  IF (KFACE_LOCAL_FOUND)

    # we need two include directories: because the version.h file is put into the build directory
    # TODO KFACE_INCLUDE_DIR sounds like it should contain only one directory...
    SET(KFACE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${KFACE_LOCAL_DIR} ${CMAKE_BINARY_DIR}/${KFACE_LOCAL_DIR})
    SET(KFACE_DEFINITIONS "-I${CMAKE_SOURCE_DIR}/${KFACE_LOCAL_DIR}" "-I${CMAKE_BINARY_DIR}/${KFACE_LOCAL_DIR}")
    SET(KFACE_LIBRARIES kface)
    MESSAGE(STATUS "Found Kface library in local sub-folder: ${CMAKE_SOURCE_DIR}/${KFACE_LOCAL_DIR}")
    SET(KFACE_FOUND TRUE)
    MARK_AS_ADVANCED(KFACE_INCLUDE_DIR KFACE_LIBRARIES KFACE_DEFINITIONS)

  ELSE(KFACE_LOCAL_FOUND)
    IF(NOT WIN32) 
      MESSAGE(STATUS "Check Kface library using pkg-config...")

      # use pkg-config to get the directories and then use these values
      # in the FIND_PATH() and FIND_LIBRARY() calls
      INCLUDE(UsePkgConfig)

      PKGCONFIG(libkface _KFACEIncDir _KFACELinkDir _KFACELinkFlags _KFACECflags)

      IF(_KFACELinkFlags)
        # query pkg-config asking for a libkface >= 0.1.0
        EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=0.1.0 libkface RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
        IF(_return_VALUE STREQUAL "0")
            MESSAGE(STATUS "Found libkface release >= 0.1.0")
            SET(KFACE_VERSION_GOOD_FOUND TRUE)
        ELSE(_return_VALUE STREQUAL "0")
            MESSAGE(STATUS "Found libkface release < 0.1.0, too old")
            SET(KFACE_VERSION_GOOD_FOUND FALSE)
            SET(KFACE_FOUND FALSE)
        ENDIF(_return_VALUE STREQUAL "0")
      ELSE(_KFACELinkFlags)
        SET(KFACE_VERSION_GOOD_FOUND FALSE)
        SET(KFACE_FOUND FALSE)
      ENDIF(_KFACELinkFlags)
    ELSE(NOT WIN32)
      SET(KFACE_VERSION_GOOD_FOUND TRUE)
    ENDIF(NOT WIN32)

    IF(KFACE_VERSION_GOOD_FOUND)
        SET(KFACE_DEFINITIONS "${_KFACECflags}")

        FIND_PATH(KFACE_INCLUDE_DIR libkface/version.h
        ${_KFACEIncDir}
        )

        FIND_LIBRARY(KFACE_LIBRARIES NAMES kface
        PATHS
        ${_KFACELinkDir}
        )

        IF (KFACE_INCLUDE_DIR AND KFACE_LIBRARIES)
            SET(KFACE_FOUND TRUE)
        ENDIF (KFACE_INCLUDE_DIR AND KFACE_LIBRARIES)
      ENDIF(KFACE_VERSION_GOOD_FOUND) 
      IF (KFACE_FOUND)
          IF (NOT Kface_FIND_QUIETLY)
              MESSAGE(STATUS "Found libkface: ${KFACE_LIBRARIES}")
          ENDIF (NOT Kface_FIND_QUIETLY)
      ELSE (KFACE_FOUND)
          IF (Kface_FIND_REQUIRED)
              IF (NOT KFACE_INCLUDE_DIR)
                  MESSAGE(FATAL_ERROR "Could NOT find libkface header files")
              ENDIF (NOT KFACE_INCLUDE_DIR)
              IF (NOT KFACE_LIBRARIES)
                  MESSAGE(FATAL_ERROR "Could NOT find libkface library")
              ENDIF (NOT KFACE_LIBRARIES)
          ENDIF (Kface_FIND_REQUIRED)
      ENDIF (KFACE_FOUND)

    MARK_AS_ADVANCED(KFACE_INCLUDE_DIR KFACE_LIBRARIES KFACE_DEFINITIONS)

  ENDIF(KFACE_LOCAL_FOUND)

ENDIF (KFACE_INCLUDE_DIR AND KFACE_LIBRARIES AND KFACE_DEFINITIONS)
