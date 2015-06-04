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

add_library(fuzzysearch STATIC ${libfuzzysearch_SRCS})
target_include_directories(fuzzysearch
                           PRIVATE
                           "$<TARGET_PROPERTY:digikamgui,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:setup,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:maintenance,INTERFACE_INCLUDE_DIRECTORIES>"
)
target_link_libraries(fuzzysearch
                      digikamalbum
                      digikamwidgets
)
