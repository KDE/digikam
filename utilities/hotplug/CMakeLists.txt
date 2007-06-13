INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/apps/konqueror/servicemenus FILES  digikam-download.desktop digikam-gphoto2-camera.desktop digikam-mount-and-download.desktop )
INSTALL_FILES(  FILES  digikam-camera )



#original Makefile.am contents follow:

#konqservicemenudir = $(kde_datadir)/konqueror/servicemenus
#konqservicemenu_DATA = digikam-download.desktop digikam-gphoto2-camera.desktop digikam-mount-and-download.desktop
#
#helperdir = $(digikamhelper_dir)
#helper_SCRIPTS = digikam-camera
#
##EXTRA_DIST = $(servicemenu_DATA) $(helper_SCRIPTS)
