SET(libcameragui_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main/cameraui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controller/cameracontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controller/camerahistoryupdater.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/devices/dkcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/devices/gpcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/devices/camiteminfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/devices/umscamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/camerafolderdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/camerainfodialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/capturedlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/cameramessagebox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/cameranamehelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/freespacetooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/capturewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/freespacewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/renamecustomizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/albumcustomizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/advancedsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/scriptingsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/filtercombo.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/models/importimagemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/importthumbnailmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/importfiltermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/importmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/camitemsortsettings.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/items/itemviewimportdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/importcategorydrawer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/items/importdelegate.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/views/importsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importcategorizedview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importthumbnailbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importpreviewview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importiconview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importiconview_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importstackedview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/views/importview.cpp

    #${CMAKE_CURRENT_SOURCE_DIR}/controller/camerathumbsctrl.cpp

    #${CMAKE_CURRENT_SOURCE_DIR}/items/camerafolderitem.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/items/cameraiconitem.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/views/camerafolderview.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/views/cameraiconview.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/views/cameraiconviewtooltip.cpp

    #${CMAKE_CURRENT_SOURCE_DIR}/q3support/icongroupitem.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/q3support/iconitem.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/q3support/iconview.cpp
)

KDE4_ADD_LIBRARY(cameragui STATIC ${libcameragui_SRCS})

#TARGET_LINK_LIBRARIES(cameragui ${QT_QT3SUPPORT_LIBRARY})

INSTALL(FILES main/cameraui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
