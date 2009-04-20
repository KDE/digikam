
INCLUDE_DIRECTORIES(../utilities/imageeditor/editor
                    ../utilities/imageeditor/canvas
                    ../libs/histogram 
                    ../libs/levels
                    ../libs/curves 
                    ../libs/greycstoration
                    ../libs/whitebalance
                    ../libs/widgets/common
                    ../libs/widgets/iccprofiles
                    ../libs/widgets/imageplugins
                    ../libs/dialogs 
                    ../libs/dimg 
                    ../libs/dimg/filters 
                    ../libs/dimg/loaders 
                    ../libs/threadimageio
                    ../libs/dmetadata 
                    ../libs/themeengine 
                    ../digikam 
                    ../
                    ${KDCRAW_INCLUDE_DIR}
                   )

ADD_SUBDIRECTORY( adjustcurves )
ADD_SUBDIRECTORY( adjustlevels )
ADD_SUBDIRECTORY( blurfx )
ADD_SUBDIRECTORY( border )
ADD_SUBDIRECTORY( channelmixer )
ADD_SUBDIRECTORY( charcoal )
ADD_SUBDIRECTORY( colorfx )
ADD_SUBDIRECTORY( coreplugin )
ADD_SUBDIRECTORY( distortionfx )
ADD_SUBDIRECTORY( emboss )
ADD_SUBDIRECTORY( filmgrain )
ADD_SUBDIRECTORY( freerotation )
ADD_SUBDIRECTORY( hotpixels )
ADD_SUBDIRECTORY( infrared )
ADD_SUBDIRECTORY( inpainting )
ADD_SUBDIRECTORY( inserttext )
ADD_SUBDIRECTORY( lenscorrection )
ADD_SUBDIRECTORY( noisereduction )
ADD_SUBDIRECTORY( oilpaint )
ADD_SUBDIRECTORY( perspective )
ADD_SUBDIRECTORY( raindrop )
ADD_SUBDIRECTORY( restoration )
ADD_SUBDIRECTORY( sheartool )
ADD_SUBDIRECTORY( superimpose )
ADD_SUBDIRECTORY( texture )
ADD_SUBDIRECTORY( whitebalance )
#ADD_SUBDIRECTORY( contentawareresizing )
