ADD_SUBDIRECTORY( imageeditor ) 
ADD_SUBDIRECTORY( splashscreen ) 
ADD_SUBDIRECTORY( setup ) 
ADD_SUBDIRECTORY( cameragui ) 
ADD_SUBDIRECTORY( hotplug ) 
ADD_SUBDIRECTORY( scripts ) 
ADD_SUBDIRECTORY( batch ) 
ADD_SUBDIRECTORY( slideshow ) 
ADD_SUBDIRECTORY( lighttable ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = imageeditor splashscreen setup cameragui hotplug scripts batch slideshow lighttable 
