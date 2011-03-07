SET(libcameragui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/cameracontroller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/camerahistoryupdater.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/devices/dkcamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/devices/gpcamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/devices/gpiteminfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/devices/umscamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/camerafolderdialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/camerainfodialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/capturedlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/items/camerafolderitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/items/cameraiconitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/views/camerafolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/views/cameraiconview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/views/cameraiconviewtooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/views/cameranamehelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/views/freespacetooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/capturewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/freespacewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/renamecustomizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/main/cameraui.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/q3support/icongroupitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/q3support/iconitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/q3support/iconview.cpp
)

KDE4_ADD_LIBRARY(cameragui STATIC ${libcameragui_SRCS})

INSTALL( FILES main/cameraui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
