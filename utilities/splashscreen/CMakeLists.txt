
include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam  )


SET(libsplashscreen_STAT_SRCS splashscreen.cpp )


kde4_automoc(${libsplashscreen_STAT_SRCS})

KDE4_ADD_LIBRARY(libsplashscreen STATIC ${libsplashscreen_STAT_SRCS})



install( FILES  digikam-splash.png DESTINATION ${DATA_INSTALL_DIR}/digikam )

