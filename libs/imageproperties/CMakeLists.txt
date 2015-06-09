#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libimageproperties_SRCS
    imagepropertiessidebar.cpp
    imagepropertiestab.cpp
    imagepropertiesmetadatatab.cpp
    imagepropertiescolorstab.cpp


   )

set(libimageversionssidebar_SRCS
    imagepropertiesversionstab.cpp
    versionstreeview.cpp
    versionsoverlays.cpp
    versionsdelegate.cpp
)

if(${KF5KGeoMap_FOUND})
    set(libimageproperties_SRCS
        ${libimageproperties_SRCS}
        imagepropertiesgpstab.cpp
        imagegpsmodelhelper.cpp
       )
endif()

set(libimagepropertiesdigikam_SRCS
    imagepropertiessidebarcamgui.cpp
    imagepropertiessidebardb.cpp
    cameraitempropertiestab.cpp
    imagedescedittab.cpp
    captionedit.cpp
)

# used to build digikamcore
add_library(imageproperties_src OBJECT ${libimageproperties_SRCS})

add_library(imagepropertiesdigikam_src OBJECT
                                      ${libimageversionssidebar_SRCS}
                                      ${libimagepropertiesdigikam_SRCS}
)
#add_library(imagepropertiesdigikam STATIC
#                                   $<TARGET_OBJECTS:imageproperties_src>
#                                   $<TARGET_OBJECTS:imagepropertiesdigikam_src>)

#if(${KF5KGeoMap_FOUND})
#    target_link_libraries(imagepropertiesdigikam PRIVATE KF5::KGeoMap)
#endif()

## TODO we need this to showfoto?
##add_library(imageproperties STATIC ${libimageproperties_SRCS})

#target_link_libraries(imagepropertiesdigikam
#                      PRIVATE
#                      digikamwidgets

#                      Qt5::Core
#                      Qt5::Gui
#                      Qt5::Widgets
#                      Qt5::Sql

#                      KF5::ConfigCore
#)
