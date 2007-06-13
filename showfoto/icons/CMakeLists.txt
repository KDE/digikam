INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/apps/showfoto/icons FILES  )

KDE3_INSTALL_ICONS( hicolor )



#original Makefile.am contents follow:

#showfotoicondir = $(kde_datadir)/showfoto/icons
#showfotoicon_ICON = AUTO
#
#KDE_ICON = showfoto
#
