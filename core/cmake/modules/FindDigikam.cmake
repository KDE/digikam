# A CMake script to find digiKam headers and libraries
#
# Once done this will define
#
#  Digikam_FOUND       - system has digiKam
#  DIGIKAM_INCLUDE_DIR - the digiKam include directory
#  DIGIKAM_LIBRARIES   - the digiKam core libraries
#
# Copyright (c) 2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

IF(DIGIKAM_INCLUDE_DIR AND DIGIKAM_LIBRARIES)

   # in cache already
   SET(DIGIKAM_FIND_QUIETLY TRUE)

ENDIF()

FIND_PATH(DIGIKAM_INCLUDE_DIR  NAMES digikam/dplugin.h)
FIND_LIBRARY(DIGIKAM_LIBRARIES NAMES digikamcore digikamgui)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Digikam DEFAULT_MSG DIGIKAM_INCLUDE_DIR DIGIKAM_LIBRARIES)

MESSAGE(STATUS "Digikam_FOUND       = ${Digikam_FOUND}")
MESSAGE(STATUS "DIGIKAM_INCLUDE_DIR = ${DIGIKAM_INCLUDE_DIR}")
MESSAGE(STATUS "DIGIKAM_LIBRARIES   = ${DIGIKAM_LIBRARIES}")
