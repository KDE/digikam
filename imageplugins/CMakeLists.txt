ADD_SUBDIRECTORY( coreplugin ) 
ADD_SUBDIRECTORY( adjustcurves ) 
ADD_SUBDIRECTORY( adjustlevels ) 
ADD_SUBDIRECTORY( antivignetting ) 
ADD_SUBDIRECTORY( blurfx ) 
ADD_SUBDIRECTORY( border ) 
ADD_SUBDIRECTORY( channelmixer ) 
ADD_SUBDIRECTORY( charcoal ) 
ADD_SUBDIRECTORY( colorfx ) 
ADD_SUBDIRECTORY( distortionfx ) 
ADD_SUBDIRECTORY( emboss ) 
ADD_SUBDIRECTORY( filmgrain ) 
ADD_SUBDIRECTORY( freerotation ) 
ADD_SUBDIRECTORY( hotpixels ) 
ADD_SUBDIRECTORY( infrared ) 
ADD_SUBDIRECTORY( inpainting ) 
ADD_SUBDIRECTORY( inserttext ) 
ADD_SUBDIRECTORY( lensdistortion ) 
ADD_SUBDIRECTORY( noisereduction ) 
ADD_SUBDIRECTORY( oilpaint ) 
ADD_SUBDIRECTORY( perspective ) 
ADD_SUBDIRECTORY( raindrop ) 
ADD_SUBDIRECTORY( restoration ) 
ADD_SUBDIRECTORY( sheartool ) 
ADD_SUBDIRECTORY( superimpose ) 
ADD_SUBDIRECTORY( texture ) 
ADD_SUBDIRECTORY( whitebalance ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = coreplugin adjustcurves adjustlevels antivignetting blurfx border \
#	      channelmixer charcoal colorfx distortionfx emboss filmgrain \
#	      freerotation hotpixels infrared inpainting inserttext lensdistortion \
#	      noisereduction oilpaint perspective raindrop restoration sheartool \
#	      superimpose texture whitebalance 
