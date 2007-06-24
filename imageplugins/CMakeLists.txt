
INCLUDE_DIRECTORIES(
		${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders
		${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
		${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                ${JPEG_INCLUDE_DIR}
                ${TIFF_INCLUDE_DIR}
                ${PNG_INCLUDE_DIR}
                ${JASPER_INCLUDE_DIR}
                   )


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

