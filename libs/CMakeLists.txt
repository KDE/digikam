ADD_SUBDIRECTORY( lprof ) 
ADD_SUBDIRECTORY( histogram ) 
ADD_SUBDIRECTORY( levels ) 
ADD_SUBDIRECTORY( curves ) 
ADD_SUBDIRECTORY( whitebalance ) 
ADD_SUBDIRECTORY( dmetadata ) 
ADD_SUBDIRECTORY( dimg ) 
ADD_SUBDIRECTORY( threadimageio ) 
ADD_SUBDIRECTORY( themeengine ) 
ADD_SUBDIRECTORY( widgets ) 
ADD_SUBDIRECTORY( greycstoration ) 
ADD_SUBDIRECTORY( thumbbar ) 
ADD_SUBDIRECTORY( jpegutils ) 
ADD_SUBDIRECTORY( imageproperties ) 
ADD_SUBDIRECTORY( dialogs ) 
ADD_SUBDIRECTORY( database ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = lprof histogram levels curves whitebalance dmetadata \
#          dimg threadimageio themeengine widgets greycstoration \
#          thumbbar jpegutils imageproperties dialogs database
