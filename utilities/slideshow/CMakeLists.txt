#Laurent Clean up it
include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(libslideshow_STAT_SRCS toolbar.cpp slideshow.cpp )
kde4_automoc(${libslideshow_STAT_SRCS})

KDE4_ADD_LIBRARY(libslideshow STATIC ${libslideshow_STAT_SRCS})

