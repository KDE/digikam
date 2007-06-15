
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############
set(libthemeengine_STAT_SRCS theme.cpp themeengine.cpp texture.cpp)
kde4_automoc(${libthemeengine_STAT_SRCS})
KDE4_ADD_LIBRARY(libthemeengine STATIC ${libthemeengine_STAT_SRCS})
