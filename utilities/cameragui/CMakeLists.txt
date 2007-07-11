
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor
                     ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils
                     ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine
                     ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database  
                     ${GPHOTO2_INCLUDE_DIR} 
                   )

SET(libcameragui_STAT_SRCS dkcamera.cpp 
                           umscamera.cpp 
                           gpcamera.cpp 
                           gpiteminfo.cpp 
                           animwidget.cpp 
                           renamecustomizer.cpp 
                           albumselectdialog.cpp 
                           camerainfodialog.cpp 
                           camerafolderdialog.cpp 
                           cameraiconview.cpp 
                           cameraiconitem.cpp 
                           camerafolderview.cpp 
                           camerafolderitem.cpp 
                           cameracontroller.cpp 
                           cameraui.cpp
   )

KDE4_AUTOMOC(${libcameragui_STAT_SRCS})

KDE4_ADD_LIBRARY(libcameragui STATIC ${libcameragui_STAT_SRCS})
