# A macro wrapper to find OpenCV library
#
# Syntax:  DETECT_OPENCV(OPENCV_MIN_VERSION OPENCV_REQUIRED_COMPONENTS) 
#
# Example: DETECT_OPENCV(2.4.9 core highgui objdetect contrib)
# which try to find OpenCV version 2.4.9 
# with internal components "core", "highgui", "objdetect", and "contrib".
#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier.gilles@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# On some systems, OpenCV can be found using FIND_PACKAGE(OpenCV), for other systems
# we ship our own version of FindOpenCV.cmake. But that does not work on all systems.
# Therefore, first try finding OpenCV using FIND_PACKAGE(OpenCV), and if that fails,
# add our FindOpenCV.cmake to the search path and search again.

MACRO(DETECT_OPENCV OPENCV_MIN_VERSION)

    # Reset to avoid picking up extra libraries
    SET(OpenCV_LIBS)

    set(OPENCV_REQUIRED_COMPONENTS "${ARGN}" )

    MESSAGE(STATUS "First try at finding OpenCV...")
    FIND_PACKAGE(OpenCV COMPONENTS ${OPENCV_REQUIRED_COMPONENTS})

    IF (NOT OpenCV_LIBRARIES AND NOT OpenCV_LIBS)

        MESSAGE(STATUS "Could not find OpenCV normally, trying internal FindOpenCV.cmake")
        SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/modules_opencv)
        FIND_PACKAGE(OpenCV REQUIRED COMPONENTS ${OPENCV_REQUIRED_COMPONENTS})

    ELSE ()

        MESSAGE(STATUS "Great, found OpenCV on the first try.")

    ENDIF ()

    MESSAGE(STATUS "OpenCV Root directory is: ${OpenCV_DIR}")

    # check OpenCV version

    IF (OpenCV_VERSION)

        MESSAGE(STATUS "OpenCV: Found version ${OpenCV_VERSION} (required: ${OPENCV_MIN_VERSION})")

        IF (${OpenCV_VERSION} VERSION_LESS ${OPENCV_MIN_VERSION})

            MESSAGE(WARNING "OpenCV: Version is too old.")
            SET(OpenCV_FOUND FALSE)

        ENDIF (${OpenCV_VERSION} VERSION_LESS ${OPENCV_MIN_VERSION})

    ELSE ()

        MESSAGE(WARNING "OpenCV: Version information not found, your version is probably too old.")
        SET(OpenCV_FOUND FALSE)

    ENDIF ()

    # There are two versions of FindOpenCV.cmake in the wild, one defining
    # OpenCV_LIBRARIES, the other defining OpenCV_LIBS. Make sure we handle
    # both cases.

    IF (NOT OpenCV_LIBRARIES)

        SET(OpenCV_LIBRARIES ${OpenCV_LIBS})

    ENDIF ()

    # Same story with OpenCV_INCLUDE_DIRS and OpenCV_INCLUDE_DIR:

    IF (NOT OpenCV_INCLUDE_DIRS)

        SET(OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIR})

    ENDIF ()

    MESSAGE(STATUS "OpenCV headers: ${OpenCV_INCLUDE_DIRS}")
    MESSAGE(STATUS "OpenCV libs   : ${OpenCV_LIBRARIES}")

ENDMACRO(DETECT_OPENCV)
