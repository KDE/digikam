SET_DEBUG_AREA_CODE(${AREA_CODE_KIOSLAVES})

INCLUDE_DIRECTORIES(
                     # database libs
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database
                     ${CMAKE_CURRENT_SOURCE_DIR}/../digikam
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dmetadata
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database/haar
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database/sqlite2

                     # config-digikam.h
                     ${CMAKE_CURRENT_BINARY_DIR}/../
                   )

# # digiKam Tags kio slave ###########################################################

SET(kio_digikamtags_PART_SRCS digikamtags.cpp )

KDE4_ADD_PLUGIN(kio_digikamtags ${kio_digikamtags_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamtags
                      digikamcore
                      digikamdatabase
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Albums kio slave #########################################################

SET(kio_digikamalbums_PART_SRCS digikamalbums.cpp )

KDE4_ADD_PLUGIN(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamalbums
                      digikamcore
                      digikamdatabase
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Date kio slave ###########################################################

SET(kio_digikamdates_PART_SRCS digikamdates.cpp)

KDE4_ADD_PLUGIN(kio_digikamdates ${kio_digikamdates_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamdates
                      digikamcore
                      digikamdatabase
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Search kio slave #########################################################

SET(kio_digikamsearch_PART_SRCS digikamsearch.cpp )


KDE4_ADD_PLUGIN(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamsearch
                      digikamcore
                      digikamdatabase
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
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
