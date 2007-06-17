
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/setup 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor 
                   )

SET(liblighttable_STAT_SRCS lighttablebar.cpp lighttablewindow.cpp lighttablepreview.cpp lighttableview.cpp)

KDE4_AUTOMOC(${liblighttable_STAT_SRCS})

KDE4_ADD_LIBRARY(liblighttable STATIC ${liblighttable_STAT_SRCS})

INSTALL( FILES lighttablewindowui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
