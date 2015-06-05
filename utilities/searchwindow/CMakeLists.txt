#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libsearchwindow_SRCS
    searchwindow.cpp
    searchview.cpp
    searchgroup.cpp
    searchfieldgroup.cpp
    searchfields.cpp
    searchutilities.cpp
    ratingsearchutilities.cpp
    choicesearchutilities.cpp
    searchfolderview.cpp
    searchtabheader.cpp
)

add_library(searchwindow STATIC ${libsearchwindow_SRCS})

target_link_libraries(searchwindow
                      PRIVATE
                      digikamdatabase_static

                      Qt5::Widgets
)
