
INCLUDE_DIRECTORIES( ../digikam
                     ../libs/jpegutils 
                     ../libs/dimg 
                     ../libs/dmetadata 
                     ../libs/database 
                     ../libs/database/sqlite2
                     ../libs/dimg/loaders
                     ../libs/dimg/filters
                     ../libs/dmetadata
                     ../libs/widgets/metadata
                     ../libs/histogram
                     ../libs/levels
                     ../libs/lprof/
                     ../libs/threadimageio
                     ${JPEG_INCLUDE_DIR}
                     ${TIFF_INCLUDE_DIR}
                     ${PNG_INCLUDE_DIR}
                     ${JASPER_INCLUDE_DIR}
                   )

# # digiKam Tags kio slave ###########################################################

SET(kio_digikamtags_PART_SRCS digikamtags.cpp )


KDE4_ADD_PLUGIN(kio_digikamtags ${kio_digikamtags_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamtags
                      digikam
                      digikamdatabase
                     )


# # digiKam Albums kio slave #########################################################

SET(kio_digikamalbums_PART_SRCS digikamalbums.cpp )

KDE4_ADD_PLUGIN(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamalbums
                      digikam
                      digikamdatabase
                     )

# # digiKam Date kio slave ###########################################################

SET(kio_digikamdates_PART_SRCS digikamdates.cpp)


KDE4_ADD_PLUGIN(kio_digikamdates ${kio_digikamdates_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamdates
                      digikam
                      digikamdatabase
                     )


# # digiKam Search kio slave #########################################################

SET(kio_digikamsearch_PART_SRCS digikamsearch.cpp )


KDE4_ADD_PLUGIN(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamsearch
                      digikam
                      digikamdatabase
                     )


# Files to install #################################################################

INSTALL( TARGETS kio_digikamtags DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamdates DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamsearch DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamalbums DESTINATION ${PLUGIN_INSTALL_DIR} )

INSTALL( FILES digikamtags.protocol
               digikamalbums.protocol
               digikamdates.protocol
               digikamsearch.protocol
         DESTINATION ${SERVICES_INSTALL_DIR}
       )
