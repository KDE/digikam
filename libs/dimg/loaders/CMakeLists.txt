
INCLUDE_DIRECTORIES(
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                   )

SET(libdimgloaders_STAT_SRCS dimgloader.cpp pngloader.cpp jpegloader.cpp
                             tiffloader.cpp rawloader.cpp ppmloader.cpp
                             qimageloader.cpp iccjpeg.c jp2kloader.cpp
                             jpegsettings.cpp pngsettings.cpp tiffsettings.cpp
                             jp2ksettings.cpp )

KDE4_AUTOMOC(${libdimgloaders_STAT_SRCS})

KDE4_ADD_LIBRARY(libdimgloaders STATIC ${libdimgloaders_STAT_SRCS})

TARGET_LINK_LIBRARIES(libdimgloaders
                      ${JPEG_LIBRARIES} 
                      ${TIFF_LIBRARIES} 
                      ${PNG_LIBRARIES} 
                      ${JASPER_LIBRARIES}
                     )

INSTALL( FILES imagelevels.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
