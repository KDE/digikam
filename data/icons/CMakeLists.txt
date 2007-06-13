INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/apps/digikam/icons FILES  )

KDE3_INSTALL_ICONS( hicolor )



#original Makefile.am contents follow:

#digikamicondir = $(kde_datadir)/digikam/icons
#digikamicon_ICON = AUTO
#
#KDE_ICON = digikam
