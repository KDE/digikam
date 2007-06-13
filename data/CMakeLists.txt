ADD_SUBDIRECTORY( profiles ) 
ADD_SUBDIRECTORY( themes ) 
ADD_SUBDIRECTORY( icons ) 
ADD_SUBDIRECTORY( pics ) 
ADD_SUBDIRECTORY( about ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = profiles themes icons pics about 
