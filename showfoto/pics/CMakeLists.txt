INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/apps/showfoto FILES  showfoto-splash.png menusidepixmap.png  )



#original Makefile.am contents follow:

#showfotosplashpicdir = $(kde_datadir)/showfoto
#showfotosplashpic_DATA = showfoto-splash.png menusidepixmap.png 
