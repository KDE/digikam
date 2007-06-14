
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${KDE4_INCLUDE_DIR} 
                     ${QT_INCLUDES}  
                   )

SET(libhistogram_STAT_SRCS imagehistogram.cpp )

KDE4_ADD_LIBRARY(libhistogram STATIC ${libhistogram_STAT_SRCS})

INSTALL( FILES imagehistogram.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )

