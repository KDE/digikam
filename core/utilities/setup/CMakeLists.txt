#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libsetupshowfoto_SRCS
    editor/setupeditoriface.cpp
    editor/setupiofiles.cpp
    editor/setupraw.cpp
    setupicc.cpp
    setupslideshow.cpp
)

set(libsetup_SRCS
    setupicc.cpp
    setup.cpp
    setuptooltip.cpp
    setuptemplate.cpp
    setupslideshow.cpp
    setupmisc.cpp
    setuplighttable.cpp
    setupdatabase.cpp
    setupimagequalitysorter.cpp
    setuputils.cpp

    editor/setupeditor.cpp
    editor/setupeditoriface.cpp
    editor/setupiofiles.cpp
    editor/setupversioning.cpp
    editor/setupraw.cpp

    album/setupalbumview.cpp
    album/setupcategory.cpp
    album/setupmime.cpp

    metadata/setupmetadata.cpp
    metadata/advancedmetadatatab.cpp
    metadata/namespacelistview.cpp
    metadata/namespaceeditdlg.cpp

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

include_directories($<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(setup_src OBJECT ${libsetup_SRCS} ${libsetupshowfoto_SRCS})

add_library(setupshowfoto_src OBJECT  ${libsetupshowfoto_SRCS})
