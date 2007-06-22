INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/lprof
                   )

SET(libiccprofileswidgets_STAT_SRCS iccpreviewwidget.cpp
                                    iccprofilewidget.cpp
                                    cietonguewidget.cpp
   )

KDE4_AUTOMOC(${libiccprofileswidgets_STAT_SRCS})

KDE4_ADD_LIBRARY(libiccprofileswidgets STATIC ${libiccprofileswidgets_STAT_SRCS})

install( FILES cietonguewidget.h 
               iccprofilewidget.h 
               iccpreviewwidget.h  
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
