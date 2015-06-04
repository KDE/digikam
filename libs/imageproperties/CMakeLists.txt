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

    # TODO these used to be somewhere else before, refactor?
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
    ${libimageproperties_SRCS}
    imagepropertiessidebarcamgui.cpp
    imagepropertiessidebardb.cpp
    cameraitempropertiestab.cpp
    imagedescedittab.cpp
    captionedit.cpp
)

add_library(imagepropertiesdigikam STATIC ${libimageproperties_SRCS} ${libimagepropertiesdigikam_SRCS})

if(${KF5KGeoMap_FOUND})
    target_link_libraries(imagepropertiesdigikam KF5::KGeoMap)
endif()

# TODO we need this to showfoto?
#add_library(imageproperties STATIC ${libimageproperties_SRCS})

target_include_directories(imagepropertiesdigikam
                           PRIVATE
                           "$<TARGET_PROPERTY:importui,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamtemplate,INTERFACE_INCLUDE_DIRECTORIES>"
)
target_link_libraries(imagepropertiesdigikam

                      digikamwidgets
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      
                      KF5::ConfigCore
)
