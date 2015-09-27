#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
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
    setupeditor.cpp
    setupmisc.cpp
    setuplighttable.cpp
    setupiofiles.cpp
    setupraw.cpp
    setupdatabase.cpp
    setupversioning.cpp
    setupimagequalitysorter.cpp

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

include_directories(
    $<TARGET_PROPERTY:KF5::Kipi,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KDcraw,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Completion,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(setup_src OBJECT ${libsetup_SRCS} ${libsetupshowfoto_SRCS})

add_library(setupshowfoto_src OBJECT  ${libsetupshowfoto_SRCS})
