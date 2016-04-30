#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:KF5::Kipi,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::WindowSystem,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

if(GPHOTO2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

set(libeditorplugin_SRCS
    plugin/imageiface.cpp
    plugin/imageplugin.cpp
    plugin/imagepluginloader.cpp
)

add_library(editorplugin_src OBJECT ${libeditorplugin_SRCS})

install(FILES plugin/digikamimageplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

set(libeditorwidgets_SRCS
    widgets/imageguidewidget.cpp
    widgets/imagepreviewitem.cpp
    widgets/previewtoolbar.cpp
    widgets/previewlist.cpp
    widgets/imageregionwidget.cpp
    widgets/imageregionitem.cpp
    widgets/rubberitem.cpp
    widgets/canvas.cpp
)

set(libeditordlg_SRCS
    dialogs/colorcorrectiondlg.cpp
    dialogs/softproofdialog.cpp
    dialogs/versioningpromptusersavedlg.cpp
)

set(libeditorgui_SRCS
    main/imagewindow.cpp
)

install(FILES main/digikamimagewindowui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)

set(libeditorcore_SRCS
    core/undocache.cpp
    core/undoaction.cpp
    core/undomanager.cpp
    core/editorcore.cpp
    core/fileoperation.cpp
    core/iccpostloadingmanager.cpp
)

set(libeditorprintiface_SRCS
    printiface/printhelper.cpp
    printiface/printoptionspage.cpp
)


ki18n_wrap_ui(libeditorprintiface_SRCS
              printiface/printoptionspage.ui
)

kconfig_add_kcfg_files(libeditorprintiface_SRCS
                       printiface/digikamconfig.kcfgc
)

set(libeditorrawimport_SRCS
    rawimport/rawimport.cpp
    rawimport/rawpreview.cpp
    rawimport/rawsettingsbox.cpp
)

set(libeditoriface_SRCS
    editor/editortool.cpp
    editor/editortooliface.cpp
    editor/editorstackview.cpp
    editor/editortoolsettings.cpp
    editor/editorwindow.cpp
    editor/actioncategorizedview.cpp
)

# this lib is used to build digikam core
add_library(imageeditor_src OBJECT ${libeditorcore_SRCS} ${libeditordlg_SRCS}  ${libeditoriface_SRCS} ${libeditorprintiface_SRCS} ${libeditorrawimport_SRCS} ${libeditorwidgets_SRCS})

add_library(imageeditorgui_src OBJECT ${libeditorgui_SRCS})

