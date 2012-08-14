SET(libimportui_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main/cameraui.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/backend/cameracontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/camerahistoryupdater.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/dkcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/gpcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/camiteminfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/umscamera.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/camerafolderdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/camerainfodialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/capturedlg.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/dialogs/cameramessagebox.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/capturewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/freespacewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/renamecustomizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/albumcustomizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/advancedsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/scriptingsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/filtercombo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/importcontextmenu.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/models/importimagemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/importthumbnailmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/importfiltermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/importmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/camitemsortsettings.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/items/camerafolderitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/itemviewimportdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/importcategorydrawer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/importdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/importtooltipfiller.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/views/camerafolderview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/cameranamehelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/freespacetooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importcategorizedview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importthumbnailbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importpreviewview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importiconview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importiconview_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importstackedview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importview.cpp
)

KDE4_ADD_LIBRARY(importui STATIC ${libimportui_SRCS})

INSTALL(FILES main/cameraui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
