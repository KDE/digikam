SET_DEBUG_AREA_CODE(${AREA_CODE_IMAGEPLUGINS})

INCLUDE_DIRECTORIES(../utilities/imageeditor/editor
                    ../utilities/imageeditor/canvas
                    ../utilities/imageeditor/widgets
                    ../libs/histogram
                    ../libs/levels
                    ../libs/curves
                    ../libs/greycstoration
                    ../libs/whitebalance
                    ../libs/widgets/common
                    ../libs/widgets/iccprofiles
                    ../libs/dialogs
                    ../libs/dimg
                    ../libs/dimg/loaders
                    ../libs/dimg/filters
                    ../libs/dimg/filters/auto
                    ../libs/dimg/filters/bcg
                    ../libs/dimg/filters/bw
                    ../libs/dimg/filters/hsl
                    ../libs/dimg/filters/fx
                    ../libs/dimg/filters/icc
                    ../libs/dimg/filters/nr
                    ../libs/dimg/filters/sharp
                    ../libs/dimg/filters/cb
                    ../libs/dimg/filters/levels
                    ../libs/dimg/filters/curves
                    ../libs/dimg/filters/wb
                    ../libs/threadimageio
                    ../libs/dmetadata
                    ../libs/themeengine
                    ../digikam
                    ../
                    ${KDCRAW_INCLUDE_DIR}
                   )

ADD_SUBDIRECTORY( blurfx )
ADD_SUBDIRECTORY( border )
ADD_SUBDIRECTORY( fxfilters )
ADD_SUBDIRECTORY( coreplugin )
ADD_SUBDIRECTORY( distortionfx )
ADD_SUBDIRECTORY( emboss )
ADD_SUBDIRECTORY( freerotation )
ADD_SUBDIRECTORY( hotpixels )
ADD_SUBDIRECTORY( inpainting )
ADD_SUBDIRECTORY( inserttext )
ADD_SUBDIRECTORY( lenscorrection )
ADD_SUBDIRECTORY( oilpaint )
ADD_SUBDIRECTORY( perspective )
ADD_SUBDIRECTORY( raindrop )
ADD_SUBDIRECTORY( restoration )
ADD_SUBDIRECTORY( sheartool )
ADD_SUBDIRECTORY( superimpose )
ADD_SUBDIRECTORY( texture )
ADD_SUBDIRECTORY( localcontrast )

IF(GLIB2_FOUND)
    ADD_SUBDIRECTORY( contentawareresizing )
ENDIF(GLIB2_FOUND)
