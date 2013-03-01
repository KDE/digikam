# - Try to find Sqlite v2
# Once done this will define:
#
#  SQLITE2_FOUND - system has Sqlite 2
#  SQLITE2_INCLUDE_DIR - the Sqlite 2 include directories
#  SQLITE2_LIBRARIES - Link these to use Sqlite 2
#  SQLITE2_DEFINITIONS - Compiler switches required for using Sqlite 2
#
# Copyright (c) 2013, Pino Toscano <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (SQLITE2_INCLUDE_DIR AND SQLITE2_LIBRARIES)
   # in cache already
   set(Sqlite2_FIND_QUIETLY TRUE)
endif ()

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if (NOT WIN32)
  find_package(PkgConfig)

  pkg_check_modules(PC_SQLITE2 QUIET sqlite)

  set(SQLITE2_DEFINITIONS ${PC_SQLITE2_CFLAGS_OTHER})
endif ()

find_path(SQLITE2_INCLUDE_DIR NAMES sqlite.h
  PATHS
  ${PC_SQLITE2_INCLUDEDIR}
  ${PC_SQLITE2_INCLUDE_DIRS}
)

find_library(SQLITE2_LIBRARIES NAMES sqlite
  PATHS
  ${PC_SQLITE2_LIBDIR}
  ${PC_SQLITE2_LIBRARY_DIRS}
)

if (SQLITE2_INCLUDE_DIR)
  set(SQLITE2_VERSION "${PC_SQLITE2_VERSION}")
  if (NOT SQLITE2_VERSION)
    file(READ ${SQLITE2_INCLUDE_DIR}/sqlite.h _sqlite_h_content LIMIT 1024)
    string(REGEX MATCH ".*# +define +SQLITE_VERSION +\"([^\"]+)\".*" _matched_version "${_sqlite_h_content}")
    if (_matched_version)
      set(SQLITE2_VERSION "${CMAKE_MATCH_1}")
    endif ()
    unset(_sqlite_h_content)
    unset(_matched_version)
  endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sqlite2
  REQUIRED_VARS SQLITE2_LIBRARIES SQLITE2_INCLUDE_DIR
  VERSION_VAR SQLITE2_VERSION
)

# show the SQLITE2_INCLUDE_DIR and SQLITE2_LIBRARIES variables only in the advanced view
mark_as_advanced(SQLITE2_INCLUDE_DIR SQLITE2_LIBRARIES)

