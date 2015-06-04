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

add_library(editorplugin STATIC ${libeditorplugin_SRCS})

target_link_libraries(editorplugin
                      dimg
)

target_include_directories(editorplugin
                           PRIVATE
                           "$<TARGET_PROPERTY:imageeditor,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:imagepropertiesdigikam,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:slideshow,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:KF5::Kipi,INTERFACE_INCLUDE_DIRECTORIES>" #kmainwindow
                           "$<TARGET_PROPERTY:digikamsettings,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamgui,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:setup,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:fileactionmanager,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamtags,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:notificationmanager,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>"
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

add_library(imageeditor STATIC ${libeditorcore_SRCS} ${libeditordlg_SRCS} ${libeditorgui_SRCS} ${libeditoriface_SRCS} ${libeditorprintiface_SRCS} ${libeditorrawimport_SRCS} ${libeditorwidgets_SRCS})

target_include_directories(imageeditor PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/printiface>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/main>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/editor>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dialogs>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/rawimport>
                           PRIVATE
                           "$<TARGET_PROPERTY:imagepropertiesdigikam,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:slideshow,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:KF5::Kipi,INTERFACE_INCLUDE_DIRECTORIES>" #kmainwindow
                           "$<TARGET_PROPERTY:digikamsettings,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamgui,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:setup,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:fileactionmanager,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamtags,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:notificationmanager,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>"
)

target_link_libraries(imageeditor
                      versionmanager
                      editorplugin
)
