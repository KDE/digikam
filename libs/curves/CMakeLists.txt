
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                   )

SET(libcurves_STAT_SRCS imagecurves.cpp )

KDE4_ADD_LIBRARY(libcurves STATIC ${libcurves_STAT_SRCS})

install( FILES imagecurves.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
