#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

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

if(${Marble_FOUND})
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
    disjointmetadata.cpp
    captionedit.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

# used to build digikamcore
add_library(imageproperties_src OBJECT ${libimageproperties_SRCS})

add_library(imagepropertiesdigikam_src OBJECT
                                      ${libimageversionssidebar_SRCS}
                                      ${libimagepropertiesdigikam_SRCS}
)
