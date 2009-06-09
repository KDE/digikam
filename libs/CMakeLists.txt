INCLUDE_DIRECTORIES( dimg
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
ADD_SUBDIRECTORY( curves )
ADD_SUBDIRECTORY( levels )
ADD_SUBDIRECTORY( histogram )
ADD_SUBDIRECTORY( dmetadata )
ADD_SUBDIRECTORY( dimg )
ADD_SUBDIRECTORY( greycstoration )
ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( widgets )
ADD_SUBDIRECTORY( database )
ADD_SUBDIRECTORY( threadimageio )
