SET_DEBUG_AREA_CODE(${AREA_CODE_KIOSLAVES})

# # digiKam Tags kio slave ###########################################################

SET(kio_digikamtags_PART_SRCS digikamtags.cpp )

KDE4_ADD_PLUGIN(kio_digikamtags ${kio_digikamtags_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamtags
                      digikamdatabase
                      digikamkioslave
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Albums kio slave #########################################################

SET(kio_digikamalbums_PART_SRCS digikamalbums.cpp )

KDE4_ADD_PLUGIN(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamalbums
                      digikamdatabase
                      digikamkioslave
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Date kio slave ###########################################################

SET(kio_digikamdates_PART_SRCS digikamdates.cpp)

KDE4_ADD_PLUGIN(kio_digikamdates ${kio_digikamdates_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamdates
                      digikamdatabase
                      digikamkioslave
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam MapImages kio slave ######################################################

SET(kio_digikammapimages_PART_SRCS digikammapimages.cpp)

KDE4_ADD_PLUGIN(kio_digikammapimages ${kio_digikammapimages_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikammapimages
                      digikamdatabase
                      digikamkioslave
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Search kio slave #########################################################

SET(kio_digikamsearch_PART_SRCS digikamsearch.cpp )

KDE4_ADD_PLUGIN(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamsearch
                      digikamdatabase
                      digikamkioslave
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# Files to install ###################################################################

INSTALL(TARGETS kio_digikamtags      DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(TARGETS kio_digikamdates     DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(TARGETS kio_digikamsearch    DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(TARGETS kio_digikamalbums    DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(TARGETS kio_digikammapimages DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamtags.protocol
              digikamalbums.protocol
              digikamdates.protocol
              digikamsearch.protocol
              digikammapimages.protocol
        DESTINATION ${SERVICES_INSTALL_DIR}
       )
