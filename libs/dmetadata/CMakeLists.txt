
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                   )

SET(libdmetadata_STAT_SRCS dmetadata.cpp )

KDE4_ADD_LIBRARY(libdmetadata STATIC ${libdmetadata_STAT_SRCS})

INSTALL( FILES dmetadata.h photoinfocontainer.h 
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
