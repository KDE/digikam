#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libtags_SRCS
    widgets/addtagscombobox.cpp
    widgets/addtagslineedit.cpp
    widgets/tagcheckview.cpp
    widgets/tagfolderview.cpp
    widgets/tagslineeditoverlay.cpp
    widgets/tagspopupmenu.cpp
    widgets/tageditdlg.cpp

    engine/tagscompleter.cpp
    engine/tagmodificationhelper.cpp
    engine/tagsactionmngr.cpp
    engine/taggingaction.cpp
    engine/taggingactionfactory.cpp

    manager/tagsmanager.cpp
    manager/tagpropwidget.cpp
    manager/taglist.cpp
    manager/tagmngrtreeview.cpp
    manager/models/tagmngrlistview.cpp
    manager/models/tagmngrlistmodel.cpp
    manager/models/tagmngrlistitem.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5IconThemes_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::IconThemes,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_library(digikamtags_src OBJECT ${libtags_SRCS})
