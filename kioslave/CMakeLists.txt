#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET_DEBUG_AREA_CODE(${AREA_CODE_KIOSLAVES})

# # digiKam Tags kio slave ###########################################################

set(kio_digikamtags_PART_SRCS digikamtags.cpp)

KDE4_ADD_PLUGIN(kio_digikamtags ${kio_digikamtags_PART_SRCS})

target_link_libraries(kio_digikamtags
                      digikamdatabase
                      digikamcore
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Albums kio slave #########################################################

set(kio_digikamalbums_PART_SRCS digikamalbums.cpp)

KDE4_ADD_PLUGIN(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})

target_link_libraries(kio_digikamalbums
                      digikamdatabase
                      digikamcore
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Date kio slave ###########################################################

set(kio_digikamdates_PART_SRCS digikamdates.cpp)

KDE4_ADD_PLUGIN(kio_digikamdates ${kio_digikamdates_PART_SRCS})

target_link_libraries(kio_digikamdates
                      digikamdatabase
                      digikamcore
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam MapImages kio slave ######################################################

set(kio_digikammapimages_PART_SRCS digikammapimages.cpp)

KDE4_ADD_PLUGIN(kio_digikammapimages ${kio_digikammapimages_PART_SRCS})

target_link_libraries(kio_digikammapimages
                      digikamdatabase
                      digikamcore
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# # digiKam Search kio slave #########################################################

set(kio_digikamsearch_PART_SRCS digikamsearch.cpp)

KDE4_ADD_PLUGIN(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})

target_link_libraries(kio_digikamsearch
                      digikamdatabase
                      digikamcore
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                     )

# Files to install ###################################################################

install(TARGETS kio_digikamtags      DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kio_digikamdates     DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kio_digikamsearch    DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kio_digikamalbums    DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kio_digikammapimages DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES digikamtags.protocol
              digikamalbums.protocol
              digikamdates.protocol
              digikamsearch.protocol
              digikammapimages.protocol
        DESTINATION ${SERVICES_INSTALL_DIR}
       )
