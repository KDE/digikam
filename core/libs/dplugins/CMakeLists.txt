#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

set(libdpluginsinterface_SRCS
    core/dplugin.cpp
    core/dpluginauthor.cpp
    core/dpluginconfview.cpp
    core/dpluginsetup.cpp
    core/dpluginaction.cpp
    core/dplugindialog.cpp
    core/dpluginaboutdlg.cpp
    core/dpluginloader.cpp
    core/dpluginloader_p.cpp
    iface/dinfointerface.cpp
    iface/dmetainfoiface.cpp
    widgets/dwizardpage.cpp
    widgets/dwizarddlg.cpp
    widgets/dsavesettingswidget.cpp
    widgets/dwizardpage.cpp
    widgets/dpreviewmanager.cpp
    widgets/dpreviewimage.cpp
    widgets/ditemslist.cpp
)

add_library(dpluginsinterface_src OBJECT ${libdpluginsinterface_SRCS})
