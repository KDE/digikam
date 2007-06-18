
INCLUDE_DIRECTORIES(   )

SET(libsplashscreen_STAT_SRCS splashscreen.cpp )

KDE4_AUTOMOC(${libsplashscreen_STAT_SRCS})

KDE4_ADD_LIBRARY(libsplashscreen STATIC ${libsplashscreen_STAT_SRCS})


