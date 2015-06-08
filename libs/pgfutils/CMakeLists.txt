#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Enable C++ Exceptions support, required by pgfutils
kde_source_files_enable_exceptions(pgfutils.cpp)

include_directories(${PGF_INCLUDE_DIRS})

set(libpgfutils_SRCS pgfutils.cpp)

# TODO needs exports to build a dynamic lib if wanted.
add_library(pgfutils_src OBJECT ${libpgfutils_SRCS})
add_library(pgfutils STATIC $<TARGET_OBJECTS:pgfutils_src>)

target_include_directories(pgfutils PUBLIC ${PGF_INCLUDE_DIRS})


target_link_libraries(pgfutils
                      PRIVATE
                      Qt5::Gui

                      ${PGF_LIBRARIES}
)
