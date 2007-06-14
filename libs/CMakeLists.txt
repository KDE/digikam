
add_subdirectory( lprof )
add_subdirectory( histogram )
add_subdirectory( levels )
add_subdirectory( curves )
add_subdirectory( whitebalance )
add_subdirectory( dmetadata )
add_subdirectory( dimg )
add_subdirectory( threadimageio )
add_subdirectory( themeengine )
add_subdirectory( widgets )
add_subdirectory( greycstoration )
add_subdirectory( thumbbar )
add_subdirectory( jpegutils )
add_subdirectory( imageproperties )
add_subdirectory( dialogs )
add_subdirectory( database )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = lprof histogram levels curves whitebalance dmetadata \
#          dimg threadimageio themeengine widgets greycstoration \
#          thumbbar jpegutils imageproperties dialogs database
