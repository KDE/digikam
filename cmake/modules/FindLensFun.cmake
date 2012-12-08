# - Try to find lensfun
# Once done this will define
#
#  LENSFUN_FOUND - system has lensfun
#  LENSFUN_INCLUDE_DIRS - the lensfun include directory
#  LENSFUN_LIBRARIES - Link these to use lensfun
#  LENSFUN_DEFINITIONS - Compiler switches required for using lensfun
#
#  Copyright (c) 2008 Adrian Schroeter <adrian@suse.de>
#  Copyright (c) 2012 Pino Toscano <pino@kde.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (LENSFUN_LIBRARIES AND LENSFUN_INCLUDE_DIRS)
  # in cache already
  set(LENSFUN_FOUND TRUE)
else (LENSFUN_LIBRARIES AND LENSFUN_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    if (LensFun_FIND_VERSION)
      set(version_string ">=${LensFun_FIND_VERSION}")
    endif()
    pkg_check_modules(PC_LENSFUN lensfun${version_string})
  else ()
    # assume it was found
    set(PC_LENSFUN_FOUND TRUE)
  endif ()

  if (PC_LENSFUN_FOUND)
    find_path(LENSFUN_INCLUDE_DIRS lensfun.h
      HINTS ${PC_LENSFUN_INCLUDE_DIRS}
      PATH_SUFFIXES lensfun
    )

    find_library(LENSFUN_LIBRARIES NAMES lensfun
      HINTS ${PC_LENSFUN_LIBRARY_DIRS}
    )

    set(LENSFUN_VERSION "${PC_LENSFUN_VERSION}")
    set(LENSFUN_DEFINITIONS ${PC_LENSFUN_CFLAGS_OTHER})
  endif ()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LensFun
                                    REQUIRED_VARS LENSFUN_INCLUDE_DIRS LENSFUN_LIBRARIES
                                    VERSION_VAR LENSFUN_VERSION
  )

  # show the LENSFUN_INCLUDE_DIRS and LENSFUN_LIBRARIES variables only in the advanced view
  mark_as_advanced(LENSFUN_INCLUDE_DIRS LENSFUN_LIBRARIES)

endif (LENSFUN_LIBRARIES AND LENSFUN_INCLUDE_DIRS)

