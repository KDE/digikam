# A macro wrapper to find OpenCV library
#
# Syntax:  DETECT_OPENCV(OPENCV_MIN_VERSION OPENCV_REQUIRED_COMPONENTS) 
#
# Example: DETECT_OPENCV(2.4.9 core highgui objdetect contrib)
# which try to find OpenCV version 2.4.9 
# with internal components "core", "highgui", "objdetect", and "contrib".
#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# On some systems, OpenCV can be found using FIND_PACKAGE(OpenCV), for other systems
# we ship our own version of FindOpenCV.cmake. But that does not work on all systems.
# Therefore, first try finding OpenCV using FIND_PACKAGE(OpenCV), and if that fails,
# add our FindOpenCV.cmake to the search path and search again.

macro(DETECT_OPENCV OPENCV_MIN_VERSION)

    # Reset to avoid picking up extra libraries
    set(OpenCV_LIBS)

    set(OPENCV_REQUIRED_COMPONENTS "${ARGN}" )

    message(STATUS "First try at finding OpenCV...")
    find_package(OpenCV COMPONENTS ${OPENCV_REQUIRED_COMPONENTS})

    if(NOT OpenCV_LIBRARIES AND NOT OpenCV_LIBS)

        message(STATUS "Could not find OpenCV normally, trying internal FindOpenCV.cmake")
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/modules_opencv)
        find_package(OpenCV REQUIRED COMPONENTS ${OPENCV_REQUIRED_COMPONENTS})

    else()

        message(STATUS "Great, found OpenCV on the first try.")

    endif()

    message(STATUS "OpenCV Root directory is: ${OpenCV_DIR}")

    # check OpenCV version

    if(OpenCV_VERSION)

        message(STATUS "OpenCV: Found version ${OpenCV_VERSION} (required: ${OPENCV_MIN_VERSION})")

        if(${OpenCV_VERSION} VERSION_LESS ${OPENCV_MIN_VERSION})

            message(WARNING "OpenCV: Version is too old.")
            set(OpenCV_FOUND FALSE)

        endif()

    else()

        message(WARNING "OpenCV: Version information not found, your version is probably too old.")
        set(OpenCV_FOUND FALSE)

    endif()

    # There are two versions of FindOpenCV.cmake in the wild, one defining
    # OpenCV_LIBRARIES, the other defining OpenCV_LIBS. Make sure we handle
    # both cases.

    if(NOT OpenCV_LIBRARIES)

        set(OpenCV_LIBRARIES ${OpenCV_LIBS})

    endif()

    # Same story with OpenCV_INCLUDE_DIRS and OpenCV_INCLUDE_DIR:

    if(NOT OpenCV_INCLUDE_DIRS)

        set(OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIR})

    endif()

    message(STATUS "OpenCV headers: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV libs   : ${OpenCV_LIBRARIES}")

endmacro()
