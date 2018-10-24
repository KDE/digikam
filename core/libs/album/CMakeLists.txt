#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libalbum_SRCS
    engine/album.cpp
    engine/albumhistory.cpp
    engine/albummodificationhelper.cpp
    engine/albumthumbnailloader.cpp
    engine/albumwatch.cpp
    widgets/albumpropsedit.cpp
    widgets/albumselectors.cpp
    widgets/albumselectcombobox.cpp
    widgets/albumselectdialog.cpp
    widgets/albumselectwidget.cpp
    widgets/albumselecttabs.cpp
    treeview/albumlabelstreeview.cpp
    treeview/abstractalbumtreeview.cpp
    treeview/abstractcountingalbumtreeview.cpp
    treeview/abstractcheckablealbumtreeview.cpp
    treeview/albumtreeview.cpp
    treeview/albumselectiontreeview.cpp
    manager/albummanager.cpp
    manager/albummanager_p.cpp
    manager/albummanager_album.cpp
    manager/albummanager_palbum.cpp
    manager/albummanager_talbum.cpp
    manager/albummanager_dalbum.cpp
    manager/albummanager_falbum.cpp
    manager/albummanager_salbum.cpp
    manager/albummanager_database.cpp
    manager/albummanager_collection.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_library(digikamalbum_src OBJECT ${libalbum_SRCS})
