
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  digikam-download.desktop digikam-gphoto2-camera.desktop digikam-mount-and-download.desktop DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus )
install( FILES  digikam-camera DESTINATION  )



#original Makefile.am contents follow:

#konqservicemenudir = $(kde_datadir)/konqueror/servicemenus
#konqservicemenu_DATA = digikam-download.desktop digikam-gphoto2-camera.desktop digikam-mount-and-download.desktop
#
#helperdir = $(digikamhelper_dir)
#helper_SCRIPTS = digikam-camera
#
##EXTRA_DIST = $(servicemenu_DATA) $(helper_SCRIPTS)
