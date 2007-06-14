
add_subdirectory( imageeditor )
add_subdirectory( splashscreen )
add_subdirectory( setup )
add_subdirectory( cameragui )
add_subdirectory( hotplug )
add_subdirectory( scripts )
add_subdirectory( batch )
add_subdirectory( slideshow )
add_subdirectory( lighttable )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = imageeditor splashscreen setup cameragui hotplug scripts batch slideshow lighttable 
