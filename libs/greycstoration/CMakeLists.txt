ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS} "-w")

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
                   )

SET(libgreycstoration_STAT_SRCS greycstorationiface.cpp greycstorationwidget.cpp )

KDE4_AUTOMOC(${libgreycstoration_STAT_SRCS})

KDE4_ADD_LIBRARY(libgreycstoration STATIC ${libgreycstoration_STAT_SRCS})

INSTALL( FILES greycstorationiface.h 
               greycstorationwidget.h 
               greycstorationsettings.h  
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
