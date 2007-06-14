
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  tag.png tag-new.png tag-properties.png tag-delete.png tag-reset.png  	         tag-folder.png tag-events.png tag-people.png tag-places.png  	         tag-addressbook.png tag-recents.png tag-assigned.png  	         albumfolder-importimages.png albumfolder-properties.png  	         albumfolder-new.png albumfolder-importdir.png  	         underexposure.png overexposure.png  DESTINATION ${DATA_INSTALL_DIR}/digikam/icons/hicolor/32x32/actions )
install( FILES  image-full.png image-selection.png  	              histogram-lin.png histogram-log.png  		          color-picker-black.png color-picker-grey.png color-picker-white.png  	              logo-digikam.png logo-gphoto.png logo-lcms.png logo-exiv2.png logo-cimg.png  	              centerwidth.png centerheight.png target.png  	              bothvert.png bothhorz.png curvefree.png curvemooth.png original.png  	              duplicatebothvert.png duplicatebothhorz.png togglemouseover.png  		          rating.png worldmap.jpg menusidepixmap.png  DESTINATION ${DATA_INSTALL_DIR}/digikam/data )



#original Makefile.am contents follow:

#digikampicdir = $(kde_datadir)/digikam/data
#digikampic_DATA = image-full.png image-selection.png \
#	              histogram-lin.png histogram-log.png \
#		          color-picker-black.png color-picker-grey.png color-picker-white.png \
#	              logo-digikam.png logo-gphoto.png logo-lcms.png logo-exiv2.png logo-cimg.png \
#	              centerwidth.png centerheight.png target.png \
#	              bothvert.png bothhorz.png curvefree.png curvemooth.png original.png \
#	              duplicatebothvert.png duplicatebothhorz.png togglemouseover.png \
#		          rating.png worldmap.jpg menusidepixmap.png 
#
#iconsdir = $(kde_datadir)/digikam/icons/hicolor/32x32/actions
#icons_DATA = tag.png tag-new.png tag-properties.png tag-delete.png tag-reset.png \
#	         tag-folder.png tag-events.png tag-people.png tag-places.png \
#	         tag-addressbook.png tag-recents.png tag-assigned.png \
#	         albumfolder-importimages.png albumfolder-properties.png \
#	         albumfolder-new.png albumfolder-importdir.png \
#	         underexposure.png overexposure.png 