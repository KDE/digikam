
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${KDE4_INCLUDE_DIR} 
                     ${QT_INCLUDES}  
                   )


# digiKam Thumbnails kio slave #####################################################

set(kio_digikamthumbnail_PART_SRCS digikamthumbnail.cpp )

kde4_automoc(${kio_digikamthumbnail_PART_SRCS})

kde4_add_plugin(kio_digikamthumbnail ${kio_digikamthumbnail_PART_SRCS})

target_link_libraries(kio_digikamthumbnail ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KEXIV2_LIBRARIES} libjpegutils libdimg )


# # digiKam Tags kio slave ###########################################################
# 
# set(kio_digikamtags_PART_SRCS digikamtags.cpp sqlitedb.cpp )
# 
# kde4_automoc(${kio_digikamtags_PART_SRCS})
# 
# kde4_add_plugin(kio_digikamtags ${kio_digikamtags_PART_SRCS})
# 
# target_link_libraries(kio_digikamtags ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )
# 
# 
# # digiKam Albums kio slave #########################################################
# 
# set(kio_digikamalbums_PART_SRCS digikamalbums.cpp sqlitedb.cpp )
# 
# kde4_automoc(${kio_digikamalbums_PART_SRCS})
# 
# kde4_add_plugin(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})
# 
# target_link_libraries(kio_digikamalbums jpegutils ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )
# 
# 
# # digiKam Date kio slave ###########################################################
# 
# set(kio_digikamdates_PART_SRCS digikamdates.cpp sqlitedb.cpp )
# 
# kde4_automoc(${kio_digikamdates_PART_SRCS})
# 
# kde4_add_plugin(kio_digikamdates ${kio_digikamdates_PART_SRCS})
# 
# target_link_libraries(kio_digikamdates ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )
# 
# 
# # digiKam Search kio slave #########################################################
# 
# set(kio_digikamsearch_PART_SRCS digikamsearch.cpp sqlitedb.cpp )
# 
# kde4_automoc(${kio_digikamsearch_PART_SRCS})
# 
# kde4_add_plugin(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})
# 
# target_link_libraries(kio_digikamsearch ${KDE4_KIO_LIBS} sqlite libdatabase libdimg )


# Files to install #################################################################

install( TARGETS kio_digikamthumbnail DESTINATION ${PLUGIN_INSTALL_DIR} )
#install( TARGETS kio_digikamtags DESTINATION ${PLUGIN_INSTALL_DIR} )
#install( TARGETS kio_digikamalbums DESTINATION ${PLUGIN_INSTALL_DIR} )
#install( TARGETS kio_digikamdates DESTINATION ${PLUGIN_INSTALL_DIR} )
#install( TARGETS kio_digikamsearch DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES digikamthumbnail.protocol 
               digikamtags.protocol
               digikamalbums.protocol 
               digikamdates.protocol  		        
               digikamsearch.protocol 
         DESTINATION ${SERVICES_INSTALL_DIR} 
       )

