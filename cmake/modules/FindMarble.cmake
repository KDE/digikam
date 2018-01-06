# - Try to find the Marble Library
# Once done this will define
#
#  Marble_FOUND       - system has Marble
#  MARBLE_INCLUDE_DIR - the Marble include directory
#  MARBLE_LIBRARIES   - the marble core libraries
#  ASTRO_LIBRARIES    - the marble astro libraries
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

IF ( MARBLE_INCLUDE_DIR AND MARBLE_LIBRARIES )

   # in cache already
   SET( MARBLE_FIND_QUIETLY TRUE )

ENDIF ( MARBLE_INCLUDE_DIR AND MARBLE_LIBRARIES )

FIND_PATH( MARBLE_INCLUDE_DIR NAMES marble/MarbleModel.h )
FIND_LIBRARY( MARBLE_LIBRARIES NAMES marblewidget-qt5 marblewidget-qt5d )
FIND_LIBRARY( ASTRO_LIBRARIES NAMES astro astrod )

INCLUDE( FindPackageHandleStandardArgs )

FIND_PACKAGE_HANDLE_STANDARD_ARGS( Marble DEFAULT_MSG MARBLE_INCLUDE_DIR MARBLE_LIBRARIES ASTRO_LIBRARIES)

MESSAGE(STATUS "Marble_FOUND       = ${Marble_FOUND}")
MESSAGE(STATUS "MARBLE_INCLUDE_DIR = ${MARBLE_INCLUDE_DIR}")
MESSAGE(STATUS "MARBLE_LIBRARIES   = ${MARBLE_LIBRARIES}")
MESSAGE(STATUS "ASTRO_LIBRARIES    = ${ASTRO_LIBRARIES}")
