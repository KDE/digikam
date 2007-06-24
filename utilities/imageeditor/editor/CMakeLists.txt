
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
                     ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/setup 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/tools   
                   )

# Static library for showfoto #################################################

set(libshowfoto_STAT_SRCS imageiface.cpp editorwindow.cpp)

KDE4_AUTOMOC(${libshowfoto_STAT_SRCS})

KDE4_ADD_LIBRARY(libshowfoto STATIC ${libshowfoto_STAT_SRCS})


# Static library for digiKam ##################################################

#SET(libdimgeditor_STAT_SRCS editorwindow.cpp imageiface.cpp imagewindow.cpp)

#KDE4_AUTOMOC(${libdimgeditor_STAT_SRCS})

#KDE4_ADD_LIBRARY(libdimgeditor STATIC ${libdimgeditor_STAT_SRCS})


# Files to install ############################################################

INSTALL( FILES imageiface.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
INSTALL( FILES digikamimageplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
INSTALL( FILES digikamimagewindowui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
