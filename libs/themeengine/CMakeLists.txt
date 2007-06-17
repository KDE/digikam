
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam )

SET(libthemeengine_STAT_SRCS theme.cpp texture.cpp themeengine.cpp)

KDE4_AUTOMOC(${libthemeengine_STAT_SRCS})

KDE4_ADD_LIBRARY(libthemeengine STATIC ${libthemeengine_STAT_SRCS})
