ADD_SUBDIRECTORY( canvas ) 
ADD_SUBDIRECTORY( tools ) 
ADD_SUBDIRECTORY( editor ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = canvas tools editor 
