# - Try to find clapack library
# Once done this will define
#
#  CLAPACK_FOUND - system has CLAPACK library
#  CLAPACK_INCLUDE_DIR - the CLAPACK include directory
#  CLAPACK_LIBRARY - the marblewidget library
#
#  copyright 2008 by Patrick Spendrin <ps_ml@gmx.de>
#  copyright 2010 by Andreas K. Huettel <mail@akhuettel.de>
#  use this file as you like
#

if(CLAPACK_INCLUDE_DIR AND CLAPACK_LIBRARY)

  # Already in cache
  set(CLAPACK_FOUND TRUE)

else(CLAPACK_INCLUDE_DIR AND CLAPACK_LIBRARY)

  find_path(CLAPACK_INCLUDE_DIR clapack.h PATHS /usr/include/clapack NO_DEFAULT_PATH)
  if(CLAPACK_INCLUDE_DIR)
     message(STATUS "Found clapack includes: ${CLAPACK_INCLUDE_DIR}")
  else(CLAPACK_INCLUDE_DIR)
     find_path(CLAPACK_INCLUDE_DIR clapack.h PATHS /usr/include/clapack)
     if(CLAPACK_INCLUDE_DIR)
        message(STATUS "Found clapack includes: ${CLAPACK_INCLUDE_DIR}")
     endif(CLAPACK_INCLUDE_DIR)
  endif(CLAPACK_INCLUDE_DIR)

  find_library(CLAPACK_LIBRARY clapack)
  if(CLAPACK_LIBRARY)
      message(STATUS "Found clapack library: ${CLAPACK_LIBRARY}")
  endif(CLAPACK_LIBRARY)

  if(CLAPACK_INCLUDE_DIR AND CLAPACK_LIBRARY)
    set(CLAPACK_FOUND TRUE)
  endif(CLAPACK_INCLUDE_DIR AND CLAPACK_LIBRARY)

  if(NOT CLAPACK_FOUND)
      message(STATUS "Could NOT find any working clapack installation")
  endif(NOT CLAPACK_FOUND)

  mark_as_advanced(CLAPACK_INCLUDE_DIR CLAPACK_LIBRARY)
endif(CLAPACK_INCLUDE_DIR AND CLAPACK_LIBRARY)
