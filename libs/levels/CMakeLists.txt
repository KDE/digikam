
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${KDE4_INCLUDE_DIR}
                     ${QT_INCLUDES}  
                   )

SET(liblevels_STAT_SRCS imagelevels.cpp )

KDE4_ADD_LIBRARY(liblevels STATIC ${liblevels_STAT_SRCS})

INSTALL( FILES imagelevels.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )

