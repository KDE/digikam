
INCLUDE_DIRECTORIES(  
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio 
                   )

SET(libslideshow_STAT_SRCS toolbar.cpp slideshow.cpp )

KDE4_AUTOMOC(${libslideshow_STAT_SRCS})

KDE4_ADD_LIBRARY(libslideshow STATIC ${libslideshow_STAT_SRCS})

