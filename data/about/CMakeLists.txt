INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/apps/digikam/about FILES  top-right-digikam.png main.html digikam.css  	bar-bottom-left.png    bar-top-right.png      bar-bottom-middle.png  bottom-left.png  	box-middle-left.png    top-left.png           bar-bottom-right.png   bottom-middle.png  	box-middle-right.png   top-middle.png         bar-middle-left.png    bottom-right.png  	box-top-left.png       bar-middle-right.png   box-bottom-left.png    box-top-middle.png  	bar-top-left.png       box-bottom-middle.png  box-top-right.png      bar-top-middle.png  	box-bottom-right.png   kde_infopage.css )



#original Makefile.am contents follow:

#about_DATA = top-right-digikam.png main.html digikam.css \
#	bar-bottom-left.png    bar-top-right.png      bar-bottom-middle.png  bottom-left.png \
#	box-middle-left.png    top-left.png           bar-bottom-right.png   bottom-middle.png \
#	box-middle-right.png   top-middle.png         bar-middle-left.png    bottom-right.png \
#	box-top-left.png       bar-middle-right.png   box-bottom-left.png    box-top-middle.png \
#	bar-top-left.png       box-bottom-middle.png  box-top-right.png      bar-top-middle.png \
#	box-bottom-right.png   kde_infopage.css
#aboutdir = $(kde_datadir)/digikam/about
#
