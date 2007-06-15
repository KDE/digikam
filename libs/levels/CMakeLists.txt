
INCLUDE_DIRECTORIES( 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                   )

SET(liblevels_STAT_SRCS imagelevels.cpp )

KDE4_ADD_LIBRARY(liblevels STATIC ${liblevels_STAT_SRCS})

INSTALL( FILES imagelevels.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )

