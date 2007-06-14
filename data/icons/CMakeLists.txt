
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  DESTINATION ${DATA_INSTALL_DIR}/digikam/icons )

kde4_install_icons( ${ICON_INSTALL_DIR} )



#original Makefile.am contents follow:

#digikamicondir = $(kde_datadir)/digikam/icons
#digikamicon_ICON = AUTO
#
#KDE_ICON = digikam
