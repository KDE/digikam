set(libeditorplugin_SRCS
    plugin/imageiface.cpp
    plugin/imageplugin.cpp
    plugin/imagepluginloader.cpp
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

add_library(imageeditor STATIC ${libeditorcore_SRCS} ${libeditordlg_SRCS} ${libeditorgui_SRCS} ${libeditoriface_SRCS} ${libeditorplugin_SRCS} ${libeditorprintiface_SRCS} ${libeditorrawimport_SRCS} ${libeditorwidgets_SRCS})

target_include_directories(imageeditor PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/printiface>
                              )

target_link_libraries(imageeditor
                      versionmanager
                      
                      #KF5::KDELibs4Support # for KMessageBox
                      #KF5::NotifyConfig # for KNotifyConfigWidget
)
