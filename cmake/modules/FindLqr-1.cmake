# - Try to find the lqr-1 library
# Once done this will define
#
#  LQR-1_FOUND - system has the lqr-1 library
#  LQR-1_INCLUDE_DIRS - the lqr-1 library include directory
#  LQR-1_LIBRARIES - Link these to use the lqr-1 library
#
#  Copyright (c) 2009 Pino Toscano <pino@kde.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (LQR-1_LIBRARIES AND LQR-1_INCLUDE_DIRS)
  # in cache already
  set(LQR-1_FOUND TRUE)
else (LQR-1_LIBRARIES AND LQR-1_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  IF (NOT WIN32)
    include(UsePkgConfig)

    pkgconfig(lqr-1 _lqrIncDir _lqrLinkDir _lqrLinkFlags _lqrCflags)
  ENDIF (NOT WIN32)

  find_path(LQR-1_INCLUDE_DIR
    NAMES
      lqr.h
    PATH_SUFFIXES
      lqr-1
    PATHS
      ${_lqrIncDir}
  )

  find_library(LQR-1_LIBRARY
    NAMES
      lqr-1
    PATHS
      ${_lqrLinkDir}
  )

  if (LQR-1_LIBRARY AND LQR-1_INCLUDE_DIR)
    find_package(GLIB2)
    if (GLIB2_FOUND)
        include(CheckCXXSourceCompiles)
        set(CMAKE_REQUIRED_INCLUDES "${LQR-1_INCLUDE_DIR}" "${GLIB2_INCLUDE_DIR}")

        check_cxx_source_compiles("
#include <lqr.h>

int main()
{
  LqrImageType t = LQR_RGB_IMAGE;
  return 0;
}
" HAVE_LQR_0_4)
        if (HAVE_LQR_0_4)
            set(LQR-1_FOUND TRUE)
        endif (HAVE_LQR_0_4)

    endif (GLIB2_FOUND)
  endif (LQR-1_LIBRARY AND LQR-1_INCLUDE_DIR)

  if (LQR-1_FOUND)
    set(LQR-1_LIBRARIES
      ${LQR-1_LIBRARIES}
      ${LQR-1_LIBRARY}
    )
    set(LQR-1_INCLUDE_DIRS
      ${LQR-1_INCLUDE_DIR}
    )
  endif (LQR-1_FOUND)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lqr-1 DEFAULT_MSG LQR-1_INCLUDE_DIRS LQR-1_LIBRARIES)

  # show the LQR-1_INCLUDE_DIRS and LQR-1_LIBRARIES variables only in the advanced view
  mark_as_advanced(LQR-1_INCLUDE_DIRS LQR-1_LIBRARIES)

endif (LQR-1_LIBRARIES AND LQR-1_INCLUDE_DIRS)

