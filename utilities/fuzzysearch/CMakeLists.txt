#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libfuzzysearch_SRCS
    fuzzysearchview.cpp
    findduplicatesview.cpp
    findduplicatesalbum.cpp
    findduplicatesalbumitem.cpp
    sketchwidget.cpp
)
add_library(fuzzysearch_src OBJECT ${libfuzzysearch_SRCS})

add_library(fuzzysearch STATIC $<TARGET_OBJECTS:fuzzysearch_src>)

target_link_libraries(fuzzysearch
                      PRIVATE
                      digikamalbum
                      digikamwidgets
                      maintenance
)
