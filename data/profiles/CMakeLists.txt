INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/apps/digikam/profiles FILES  adobergb.icm applergb.icm srgb.icm srgb-d65.icm widegamut.icm prophoto.icm  )



#original Makefile.am contents follow:

#digikamprofilesdir = $(kde_datadir)/digikam/profiles
#digikamprofiles_DATA = adobergb.icm applergb.icm srgb.icm srgb-d65.icm widegamut.icm prophoto.icm 
