
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${KDE4_INCLUDE_DIR} 
                     ${QT_INCLUDES}  
                   )


# digiKam Thumbnails kio slave #####################################################

SET(kio_digikamthumbnail_PART_SRCS digikamthumbnail.cpp )

KDE4_AUTOMOC(${kio_digikamthumbnail_PART_SRCS})

KDE4_ADD_PLUGIN(kio_digikamthumbnail ${kio_digikamthumbnail_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamthumbnail ${KDE4_KIO_LIBS} ${KEXIV2_LIBRARIES} libjpegutils libdimg )


# # digiKam Tags kio slave ###########################################################
# 
# SET(kio_digikamtags_PART_SRCS digikamtags.cpp sqlitedb.cpp )
# 
# KDE4_AUTOMOC(${kio_digikamtags_PART_SRCS})
# 
# KDE4_ADD_PLUGIN(kio_digikamtags ${kio_digikamtags_PART_SRCS})
# 
# TARGET_LINK_LIBRARIES(kio_digikamtags ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )
# 
# 
# # digiKam Albums kio slave #########################################################
# 
# SET(kio_digikamalbums_PART_SRCS digikamalbums.cpp sqlitedb.cpp )
# 
# KDE4_AUTOMOC(${kio_digikamalbums_PART_SRCS})
# 
# KDE4_ADD_PLUGIN(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})
# 
# TARGET_LINK_LIBRARIES(kio_digikamalbums jpegutils ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )
# 
# 
# # digiKam Date kio slave ###########################################################
# 
# SET(kio_digikamdates_PART_SRCS digikamdates.cpp sqlitedb.cpp )
# 
# KDE4_AUTOMOC(${kio_digikamdates_PART_SRCS})
# 
# KDE4_ADD_PLUGIN(kio_digikamdates ${kio_digikamdates_PART_SRCS})
# 
# TARGET_LINK_LIBRARIES(kio_digikamdates ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )
# 
# 
# # digiKam Search kio slave #########################################################
# 
# SET(kio_digikamsearch_PART_SRCS digikamsearch.cpp sqlitedb.cpp )
# 
# KDE4_AUTOMOC(${kio_digikamsearch_PART_SRCS})
# 
# KDE4_ADD_PLUGIN(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})
# 
# TARGET_LINK_LIBRARIES(kio_digikamsearch ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )


# Files to install #################################################################

INSTALL( TARGETS kio_digikamthumbnail DESTINATION ${PLUGIN_INSTALL_DIR} )
#INSTALL( TARGETS kio_digikamtags DESTINATION ${PLUGIN_INSTALL_DIR} )
#INSTALL( TARGETS kio_digikamalbums DESTINATION ${PLUGIN_INSTALL_DIR} )
#INSTALL( TARGETS kio_digikamdates DESTINATION ${PLUGIN_INSTALL_DIR} )
#INSTALL( TARGETS kio_digikamsearch DESTINATION ${PLUGIN_INSTALL_DIR} )

INSTALL( FILES digikamthumbnail.protocol 
               digikamtags.protocol
               digikamalbums.protocol 
               digikamdates.protocol  		        
               digikamsearch.protocol 
         DESTINATION ${SERVICES_INSTALL_DIR} 
       )

