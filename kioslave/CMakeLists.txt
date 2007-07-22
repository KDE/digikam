
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database/sqlite2
                     ${CMAKE_SOURCE_DIR}/digikam/digikam
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata
                     ${CMAKE_SOURCE_DIR}/digikam/libs/histogram
                     ${CMAKE_SOURCE_DIR}/digikam/libs/levels
                     ${CMAKE_SOURCE_DIR}/digikam/libs/lprof/
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio
                     ${JPEG_INCLUDE_DIR}
                     ${TIFF_INCLUDE_DIR}
                     ${PNG_INCLUDE_DIR}
                     ${JASPER_INCLUDE_DIR}
                   )


# digiKam Thumbnails kio slave #####################################################

SET(kio_digikamthumbnail_PART_SRCS digikamthumbnail.cpp  )


KDE4_ADD_PLUGIN(kio_digikamthumbnail ${kio_digikamthumbnail_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamthumbnail
                      digikam
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

INSTALL( TARGETS kio_digikamthumbnail DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamtags DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamdates DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamsearch DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamalbums DESTINATION ${PLUGIN_INSTALL_DIR} )

INSTALL( FILES digikamthumbnail.protocol
               digikamtags.protocol
               digikamalbums.protocol
               digikamdates.protocol
               digikamsearch.protocol
         DESTINATION ${SERVICES_INSTALL_DIR}
       )

