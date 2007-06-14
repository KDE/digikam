
add_subdirectory( canvas )
add_subdirectory( tools )
add_subdirectory( editor )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = canvas tools editor 
