SET_DEBUG_AREA_CODE(${AREA_CODE_IMAGEPLUGINS})

INCLUDE_DIRECTORIES(../utilities/imageeditor/editor
                    ../utilities/imageeditor/canvas
                    ../utilities/imageeditor/widgets
                    ../libs/histogram
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
                    ../libs/dimg/filters/greycstoration
                    ../libs/dimg/filters/decorate
                    ../libs/dimg/filters/fx
                    ../libs/dimg/filters/icc
                    ../libs/dimg/filters/lc
                    ../libs/dimg/filters/nr
                    ../libs/dimg/filters/sharp
                    ../libs/dimg/filters/cb
                    ../libs/dimg/filters/levels
                    ../libs/dimg/filters/curves
                    ../libs/dimg/filters/wb
                    ../libs/dimg/filters/transform
                    ../libs/dimg/filters/lens
                    ../libs/threadimageio
                    ../libs/dmetadata
                    ../libs/themeengine
                    ../digikam
                    ../
                    ${KDCRAW_INCLUDE_DIR}
                   )

ADD_SUBDIRECTORY( color )
ADD_SUBDIRECTORY( decorate )
ADD_SUBDIRECTORY( enhance )
ADD_SUBDIRECTORY( filters )
ADD_SUBDIRECTORY( transform )

