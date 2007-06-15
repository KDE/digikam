
include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam  )


SET(libsplashscreen_STAT_SRCS splashscreen.cpp )

KDE4_ADD_LIBRARY(libsplashscreen STATIC ${libsplashscreen_STAT_SRCS})



install( FILES  digikam-splash.png DESTINATION ${DATA_INSTALL_DIR}/digikam )

