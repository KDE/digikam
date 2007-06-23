INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                   )

SET(libmetadatawidgets_STAT_SRCS mdkeylistviewitem.cpp
                                 metadatalistview.cpp
                                 metadatalistviewitem.cpp
                                 worldmapwidget.cpp
#                                 metadatawidget.cpp
#                                 iptcwidget.cpp
                                 exifwidget.cpp
                                 makernotewidget.cpp
                                 gpswidget.cpp
   )

KDE4_AUTOMOC(${libmetadatawidgets_STAT_SRCS})

KDE4_ADD_LIBRARY(libmetadatawidgets STATIC ${libmetadatawidgets_STAT_SRCS})

install( FILES exifwidget.h  
               iptcwidget.h 
               mdkeylistviewitem.h  
               metadatalistviewitem.h  
               worldmapwidget.h
               gpswidget.h 
               makernotewidget.h  
               metadatalistview.h 
               metadatawidget.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
