
include_directories( 
                    ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                   )

SET(libthumbbar_STAT_SRCS thumbbar.cpp thumbnailjob.cpp )

KDE4_AUTOMOC(${libthumbbar_STAT_SRCS})

KDE4_ADD_LIBRARY(libthumbbar STATIC ${libthumbbar_STAT_SRCS})

INSTALL( FILES thumbbar.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
