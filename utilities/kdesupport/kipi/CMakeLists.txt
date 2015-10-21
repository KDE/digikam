#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015 by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libkipiiface_SRCS
    kipiimagecollection.cpp
    kipiimagecollectionselector.cpp
    kipiuploadwidget.cpp
    kipiinterface.cpp
    kipiimageinfo.cpp
    kipipluginloader.cpp
)

include_directories(
    $<TARGET_PROPERTY:KF5::Kipi,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Completion,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(kipiiface_src OBJECT ${libkipiiface_SRCS})
