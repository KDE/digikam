
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  adobergb.icm applergb.icm srgb.icm srgb-d65.icm widegamut.icm prophoto.icm  DESTINATION ${DATA_INSTALL_DIR}/digikam/profiles )



#original Makefile.am contents follow:

#digikamprofilesdir = $(kde_datadir)/digikam/profiles
#digikamprofiles_DATA = adobergb.icm applergb.icm srgb.icm srgb-d65.icm widegamut.icm prophoto.icm 
