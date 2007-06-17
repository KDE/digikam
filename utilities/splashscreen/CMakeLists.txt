
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/digikam  )

SET(libsplashscreen_STAT_SRCS splashscreen.cpp )

KDE4_AUTOMOC(${libsplashscreen_STAT_SRCS})

KDE4_ADD_LIBRARY(libsplashscreen STATIC ${libsplashscreen_STAT_SRCS})

INSTALL( FILES digikam-splash.png DESTINATION ${DATA_INSTALL_DIR}/digikam )

