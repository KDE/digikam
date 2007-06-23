INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                   )

SET(libimagepluginswidgets_STAT_SRCS listboxpreviewitem.cpp
                                     imagepaniconwidget.cpp
                                     imageregionwidget.cpp
                                     imagewidget.cpp
                                     imagepannelwidget.cpp
                                     imageguidewidget.cpp
   )

KDE4_AUTOMOC(${libimagepluginswidgets_STAT_SRCS})

KDE4_ADD_LIBRARY(libimagepluginswidgets STATIC ${libimagepluginswidgets_STAT_SRCS})

install( FILES imageregionwidget.h 
               imagepaniconwidget.h
               imagepannelwidget.h 
               imageguidewidget.h
               listboxpreviewitem.h 
               imagewidget.h  
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )

