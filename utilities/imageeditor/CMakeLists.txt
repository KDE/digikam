#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libeditorplugin_SRCS
    plugin/imageiface.cpp
    plugin/imageplugin.cpp
    plugin/imagepluginloader.cpp
)

add_library(editorplugin_src OBJECT ${libeditorplugin_SRCS})

add_library(editorplugin STATIC $<TARGET_OBJECTS:editorplugin_src>)

target_link_libraries(editorplugin
                      PRIVATE
                      dimg
)

target_include_directories(editorplugin INTERFACE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/plugin>"

)

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


add_library(imageeditor STATIC $<TARGET_OBJECTS:imageeditor_src> ${libeditorgui_SRCS} )

target_include_directories(imageeditor PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/printiface>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/main>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/editor>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dialogs>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rawimport>
)

target_link_libraries(imageeditor
                      PRIVATE
                      versionmanager
                      editorplugin
                      digikamdialogs
                      digikamsettings
)
