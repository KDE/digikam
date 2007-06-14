
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  showfoto-splash.png menusidepixmap.png  DESTINATION ${DATA_INSTALL_DIR}/showfoto )



#original Makefile.am contents follow:

#showfotosplashpicdir = $(kde_datadir)/showfoto
#showfotosplashpic_DATA = showfoto-splash.png menusidepixmap.png 
