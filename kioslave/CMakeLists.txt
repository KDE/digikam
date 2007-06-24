
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database 
		     ${CMAKE_SOURCE_DIR}/digikam/libs/database/sqlite2
                     ${CMAKE_SOURCE_DIR}/digikam/digikam
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                     ${JPEG_INCLUDE_DIR}
                     ${TIFF_INCLUDE_DIR}
                     ${PNG_INCLUDE_DIR}
                     ${JASPER_INCLUDE_DIR}
                   )


# digiKam Thumbnails kio slave #####################################################

SET(kio_digikamthumbnail_PART_SRCS digikamthumbnail.cpp ${libjpegutils_SRCS} ${libdimg_SRCS})

KDE4_AUTOMOC(${kio_digikamthumbnail_PART_SRCS})

KDE4_ADD_PLUGIN(kio_digikamthumbnail ${kio_digikamthumbnail_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamthumbnail ${KDE4_KIO_LIBS} ${KEXIV2_LIBRARIES} )


# # digiKam Tags kio slave ###########################################################

SET(kio_digikamtags_PART_SRCS digikamtags.cpp ${libdatabase_SRCS} ${libdimg_SRCS})

KDE4_AUTOMOC(${kio_digikamtags_PART_SRCS})

KDE4_ADD_PLUGIN(kio_digikamtags ${kio_digikamtags_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamtags ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KIO_LIBS} )


# # digiKam Albums kio slave #########################################################

#SET(kio_digikamalbums_PART_SRCS digikamalbums.cpp)

#KDE4_AUTOMOC(${kio_digikamalbums_PART_SRCS})

#KDE4_ADD_PLUGIN(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})

#TARGET_LINK_LIBRARIES(kio_digikamalbums jpegutils ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )


# # digiKam Date kio slave ###########################################################

SET(kio_digikamdates_PART_SRCS digikamdates.cpp ${libdatabase_SRCS} ${libdimg_SRCS} ${libsqlite2_SRCS})

KDE4_AUTOMOC(${kio_digikamdates_PART_SRCS})

KDE4_ADD_PLUGIN(kio_digikamdates ${kio_digikamdates_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamdates ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KIO_LIBS} )


# # digiKam Search kio slave #########################################################

SET(kio_digikamsearch_PART_SRCS digikamsearch.cpp ${libdatabase_SRCS} ${libdimg_SRCS} ${libsqlite2_SRCS} )

KDE4_AUTOMOC(${kio_digikamsearch_PART_SRCS})

KDE4_ADD_PLUGIN(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamsearch ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KIO_LIBS})


# Files to install #################################################################

INSTALL( TARGETS kio_digikamthumbnail DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamtags DESTINATION ${PLUGIN_INSTALL_DIR} )
#INSTALL( TARGETS kio_digikamalbums DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamdates DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS kio_digikamsearch DESTINATION ${PLUGIN_INSTALL_DIR} )

INSTALL( FILES digikamthumbnail.protocol
               digikamtags.protocol
               digikamalbums.protocol
               digikamdates.protocol
               digikamsearch.protocol
         DESTINATION ${SERVICES_INSTALL_DIR}
       )

