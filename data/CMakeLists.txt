
add_subdirectory( profiles )
add_subdirectory( themes )
add_subdirectory( icons )
add_subdirectory( pics )
add_subdirectory( about )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = profiles themes icons pics about 
