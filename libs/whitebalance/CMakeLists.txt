
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${KDE4_INCLUDE_DIR} 
                     ${QT_INCLUDES}  
                   )

SET(libwhitebalance_STAT_SRCS whitebalance.cpp )

KDE4_ADD_LIBRARY(libwhitebalance STATIC ${libwhitebalance_STAT_SRCS})

INSTALL( FILES whitebalance.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )

