
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  DESTINATION ${DATA_INSTALL_DIR}/showfoto/icons )

kde4_install_icons( ${ICON_INSTALL_DIR} )



#original Makefile.am contents follow:

#showfotoicondir = $(kde_datadir)/showfoto/icons
#showfotoicon_ICON = AUTO
#
#KDE_ICON = showfoto
#
