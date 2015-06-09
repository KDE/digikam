#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libsetupshowfoto_SRCS
    setupeditor.cpp
    setupiofiles.cpp
    setupraw.cpp
    setupicc.cpp
    setupslideshow.cpp
)

set(libsetup_SRCS
    setupicc.cpp
    setupcategory.cpp
    setupalbumview.cpp
    setup.cpp
    setuptooltip.cpp
    setuptemplate.cpp
    setupmime.cpp
    setupslideshow.cpp
    setupmetadata.cpp
    setupeditor.cpp
    setupmisc.cpp
    setuplighttable.cpp
    setupiofiles.cpp
    setupraw.cpp
    setupdatabase.cpp
    setupversioning.cpp
    setupimagequalitysorter.cpp

    collections/setupcollections.cpp
    collections/setupcollectionview.cpp
    collections/dwitemdelegate.cpp
    collections/dwitemdelegate_p.cpp
    collections/dwitemdelegatepool.cpp

    camera/importfilters.cpp
    camera/setupcamera.cpp
    camera/cameraselection.cpp
    camera/cameralist.cpp
    camera/cameratype.cpp

)

add_library(setup_src OBJECT ${libsetup_SRCS} ${libsetupshowfoto_SRCS})

add_library(setupshowfoto_src OBJECT  ${libsetupshowfoto_SRCS})

#add_library(setup STATIC $<TARGET_OBJECTS:setup_src>)

## TODO may be better if users of setup would use complete path for includes?
#target_include_directories(setup PUBLIC
#                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/camera>
#                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/collections>
#)

#target_link_libraries(setup
#                      PRIVATE
#                      digikamtemplate
#                      digikamwidgets # TODO for fullscreensettings
#                      maintenance
#                      importui
#                      slideshow
#                      dimg

#                      Qt5::Widgets

#)
