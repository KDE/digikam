#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libtags_SRCS
    addtagscombobox.cpp
    tagscompleter.cpp
    addtagslineedit.cpp
    tagcheckview.cpp
    tagfolderview.cpp
    taggingaction.cpp
    tagslineeditoverlay.cpp
    tagspopupmenu.cpp
    tageditdlg.cpp
    tagmodificationhelper.cpp
    tagsactionmngr.cpp
    taggingactionfactory.cpp

    tagsmanager/tagsmanager.cpp
    tagsmanager/tagpropwidget.cpp
    tagsmanager/taglist.cpp
    tagsmanager/tagmngrtreeview.cpp
    tagsmanager/models/tagmngrlistview.cpp
    tagsmanager/models/tagmngrlistmodel.cpp
    tagsmanager/models/tagmngrlistitem.cpp
)

include_directories(tagsmanager/
                    tagsmanager/models/
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
