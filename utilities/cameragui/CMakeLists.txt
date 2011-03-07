SET(libcameragui_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/cameracontroller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camerafolderdialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camerafolderitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camerafolderview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camerahistoryupdater.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cameraiconitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cameraiconview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cameraiconviewtooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camerainfodialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cameranamehelper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cameraui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/capturedlg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/capturewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dkcamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/freespacetooltip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/freespacewidget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gpcamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gpiteminfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/renamecustomizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/umscamera.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/q3support/icongroupitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/q3support/iconitem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/q3support/iconview.cpp
)

KDE4_ADD_LIBRARY(cameragui STATIC ${libcameragui_SRCS})

INSTALL( FILES cameraui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
