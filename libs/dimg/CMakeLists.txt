
ADD_SUBDIRECTORY( loaders )
ADD_SUBDIRECTORY( filters )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                   )

SET(libdimg_STAT_SRCS dimg.cpp dimgscale.cpp dcolor.cpp dcolorcomposer.cpp ddebug.cpp fastscale.cpp )


KDE4_ADD_LIBRARY(libdimg STATIC ${libdimg_STAT_SRCS})

TARGET_LINK_LIBRARIES(libdimg 
                      libhistogram
                      liblevels
                      libcurves
                      libwhitebalance
                      libdmetadata
                      libdimgloaders
                      libdimgfilters 
                      ${KDCRAW_LIBRARIES} 
                      ${LCMS_LIBRARIES}
                     )

INSTALL( FILES dimg.h dcolor.h dcolorpixelaccess.h dcolorcomposer.h dcolorblend.h ddebug.h 
         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
