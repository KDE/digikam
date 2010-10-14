INCLUDE_DIRECTORIES( dimg
                     dimg/filters
                     dimg/loaders
                     dmetadata
                     jpegutils
                     threadimageio
                     versionmanager
                     widgets/common
                     widgets/imageplugins
                     widgets/metadata
                     widgets/iccprofiles
                     imageproperties
                     dialogs
                     3rdparty/cimg
                     3rdparty/clapack
                     3rdparty/lprof
                     3rdparty/greycstoration
                     ${JPEG_INCLUDE_DIR}
                     ${TIFF_INCLUDE_DIR}
                     ${PNG_INCLUDE_DIR}
                     ${JASPER_INCLUDE_DIR}
                   )

ADD_SUBDIRECTORY( dmetadata )
ADD_SUBDIRECTORY( dimg )
ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( widgets )
ADD_SUBDIRECTORY( database )
ADD_SUBDIRECTORY( versionmanager )