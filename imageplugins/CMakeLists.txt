
add_subdirectory( coreplugin )
add_subdirectory( adjustcurves )
add_subdirectory( adjustlevels )
add_subdirectory( antivignetting )
add_subdirectory( blurfx )
add_subdirectory( border )
add_subdirectory( channelmixer )
add_subdirectory( charcoal )
add_subdirectory( colorfx )
add_subdirectory( distortionfx )
add_subdirectory( emboss )
add_subdirectory( filmgrain )
add_subdirectory( freerotation )
add_subdirectory( hotpixels )
add_subdirectory( infrared )
add_subdirectory( inpainting )
add_subdirectory( inserttext )
add_subdirectory( lensdistortion )
add_subdirectory( noisereduction )
add_subdirectory( oilpaint )
add_subdirectory( perspective )
add_subdirectory( raindrop )
add_subdirectory( restoration )
add_subdirectory( sheartool )
add_subdirectory( superimpose )
add_subdirectory( texture )
add_subdirectory( whitebalance )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = coreplugin adjustcurves adjustlevels antivignetting blurfx border \
#	      channelmixer charcoal colorfx distortionfx emboss filmgrain \
#	      freerotation hotpixels infrared inpainting inserttext lensdistortion \
#	      noisereduction oilpaint perspective raindrop restoration sheartool \
#	      superimpose texture whitebalance 
