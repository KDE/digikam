#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libdialogs_SRCS
    dprogressdlg.cpp
    dbusydlg.cpp
    filesaveoptionsdlg.cpp
    iccprofileinfodlg.cpp
    imagedialog.cpp
    infodlg.cpp
    libsinfodlg.cpp
    rawcameradlg.cpp
    dconfigdlg.cpp
    dconfigdlgmodels.cpp
    dconfigdlgview.cpp
    dconfigdlgview_p.cpp
    dconfigdlgwidgets.cpp
    dmessagebox.cpp
    dsplashscreen.cpp
    webbrowserdlg.cpp
)

set(libdeletedialog_SRCS
    deletedialog.cpp
)

if(LensFun_FOUND)
    include_directories(${LENSFUN_INCLUDE_DIRS})
endif()

include_directories(
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_QWEBENGINE)
    include_directories($<TARGET_PROPERTY:Qt5::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
else()
    include_directories($<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

#used by digikamcore
add_library(digikamdialogscore_src OBJECT ${libdialogs_SRCS})

add_library(digikamdeletedialog_src OBJECT ${libdeletedialog_SRCS})

add_library(digikamdialogs_src OBJECT ${libdialogs_SRCS} ${libdeletedialog_SRCS})
