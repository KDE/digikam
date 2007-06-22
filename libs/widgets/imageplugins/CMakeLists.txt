INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                   )

SET(libimagepluginswidgets_STAT_SRCS imageregionwidget.cpp
                                     imagepaniconwidget.cpp
                                     imageguidewidget.cpp
                                     imagepannelwidget.cpp
                                     imagewidget.cpp
                                     listboxpreviewitem.cpp
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

