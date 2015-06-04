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

target_include_directories(searchwindow
                           PRIVATE
                           "$<TARGET_PROPERTY:KF5::KDcraw,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>"

                           "$<TARGET_PROPERTY:digikamgui,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamwidgets,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamalbum,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamsettings,INTERFACE_INCLUDE_DIRECTORIES>"

)
target_link_libraries(searchwindow
                      PRIVATE
                      digikamdatabase

                      Qt5::Widgets
)
