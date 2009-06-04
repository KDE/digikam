include_directories( dimg
                     dimg/filters
                     dimg/filters/clapack
                     dimg/loaders
                     whitebalance
                     dmetadata 
                     histogram
                     curves
                     levels
                     lprof/
                     jpegutils 
                     greycstoration
                     threadimageio
                     widgets/common
                     widgets/imageplugins 
                     widgets/metadata 
                     widgets/iccprofiles 
                     imageproperties
                     dialogs
                     ${JPEG_INCLUDE_DIR}
                     ${TIFF_INCLUDE_DIR}
                     ${PNG_INCLUDE_DIR}
                     ${JASPER_INCLUDE_DIR}
                   )
add_subdirectory( curves )
add_subdirectory( levels )
add_subdirectory( histogram )
add_subdirectory( dmetadata )
add_subdirectory( dimg )
add_subdirectory( greycstoration )
add_subdirectory( dialogs )
add_subdirectory( widgets )
add_subdirectory( database )
