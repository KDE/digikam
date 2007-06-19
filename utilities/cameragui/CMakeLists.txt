
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/database   )


########### next target ###############

SET(libcameragui_STAT_SRCS gpcamera.cpp cameraui.cpp cameraiconview.cpp cameraiconitem.cpp cameracontroller.cpp camerafolderview.cpp camerafolderitem.cpp animwidget.cpp renamecustomizer.cpp dkcamera.cpp umscamera.cpp gpiteminfo.cpp camerainfodialog.cpp albumselectdialog.cpp camerafolderdialog.cpp )

KDE4_AUTOMOC(${libcameragui_STAT_SRCS})

KDE4_ADD_LIBRARY(libcameragui STATIC ${libcameragui_STAT_SRCS})
