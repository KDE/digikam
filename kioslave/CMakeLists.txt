#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# # digiKam Tags kio slave ###########################################################

set(kio_digikamtags_PART_SRCS digikamtags.cpp)

add_library(kio_digikamtags MODULE ${kio_digikamtags_PART_SRCS})

target_link_libraries(kio_digikamtags
                      digikamdatabase
                      digikamcore
                      KF5::KDELibs4Support
                      KF5::KIOCore
                      Qt5::Core
)

# # digiKam Albums kio slave #########################################################

set(kio_digikamalbums_PART_SRCS digikamalbums.cpp)

add_library(kio_digikamalbums MODULE ${kio_digikamalbums_PART_SRCS})

target_link_libraries(kio_digikamalbums
                      digikamdatabase
                      digikamcore
                      KF5::KDELibs4Support
                      KF5::KIOCore
                      Qt5::Core
)

# # digiKam Date kio slave ###########################################################

set(kio_digikamdates_PART_SRCS digikamdates.cpp)

add_library(kio_digikamdates MODULE ${kio_digikamdates_PART_SRCS})

target_link_libraries(kio_digikamdates
                      digikamdatabase
                      digikamcore
                      KF5::KDELibs4Support
                      KF5::KIOCore
                      Qt5::Core
)

# # digiKam MapImages kio slave ######################################################

set(kio_digikammapimages_PART_SRCS digikammapimages.cpp)

add_library(kio_digikammapimages MODULE ${kio_digikammapimages_PART_SRCS})

target_link_libraries(kio_digikammapimages
                      digikamdatabase
                      digikamcore
                      KF5::KDELibs4Support
                      KF5::KIOCore
                      Qt5::Core
)

# # digiKam Search kio slave #########################################################

set(kio_digikamsearch_PART_SRCS digikamsearch.cpp)

add_library(kio_digikamsearch MODULE ${kio_digikamsearch_PART_SRCS})

target_link_libraries(kio_digikamsearch
                      digikamdatabase
                      digikamcore
                      KF5::KDELibs4Support
                      KF5::KIOCore
                      Qt5::Core
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
