
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor
                    ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas
                    ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                    ${CMAKE_SOURCE_DIR}/digikam/libs/levels
                    ${CMAKE_SOURCE_DIR}/digikam/libs/curves 
                    ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance
                    ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common
                    ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles
                    ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins
                    ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs 
                    ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                    ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters 
                    ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders 
                    ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                    ${CMAKE_SOURCE_DIR}/digikam/digikam   
                   )

# ADD_SUBDIRECTORY( adjustcurves )
# ADD_SUBDIRECTORY( adjustlevels )
# ADD_SUBDIRECTORY( antivignetting )
# ADD_SUBDIRECTORY( blurfx )
# ADD_SUBDIRECTORY( border )
# ADD_SUBDIRECTORY( channelmixer )
# ADD_SUBDIRECTORY( charcoal )
# ADD_SUBDIRECTORY( colorfx )
# ADD_SUBDIRECTORY( coreplugin )
# ADD_SUBDIRECTORY( distortionfx )
# ADD_SUBDIRECTORY( emboss )
# ADD_SUBDIRECTORY( filmgrain )
ADD_SUBDIRECTORY( freerotation )
# ADD_SUBDIRECTORY( hotpixels )
# ADD_SUBDIRECTORY( infrared )
# ADD_SUBDIRECTORY( inpainting )
# ADD_SUBDIRECTORY( inserttext )
# ADD_SUBDIRECTORY( lensdistortion )
# ADD_SUBDIRECTORY( noisereduction )
# ADD_SUBDIRECTORY( oilpaint )
# ADD_SUBDIRECTORY( perspective )
# ADD_SUBDIRECTORY( raindrop )
# ADD_SUBDIRECTORY( restoration )
# ADD_SUBDIRECTORY( sheartool )
# ADD_SUBDIRECTORY( superimpose )
# ADD_SUBDIRECTORY( texture )
ADD_SUBDIRECTORY( whitebalance )
