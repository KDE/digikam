INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/apps/digikam/themes FILES  Dreary Marine Clean OrangeCrush Sandy Dark Dessert Digikasa Gray Blue DLR  )



#original Makefile.am contents follow:

#digikamthemesdir = $(kde_datadir)/digikam/themes
#digikamthemes_DATA = Dreary Marine Clean OrangeCrush Sandy Dark Dessert Digikasa Gray Blue DLR 
