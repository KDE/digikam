#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# # digiKam Albums kio slave #########################################################

set(kio_digikamalbums_PART_SRCS digikamalbums.cpp)

add_library(kio_digikamalbums MODULE ${kio_digikamalbums_PART_SRCS})

target_link_libraries(kio_digikamalbums
                      digikamdatabase
                      digikamcore

                      KF5::KIOCore
                      KF5::I18n
                      Qt5::Core
                      Qt5::DBus
)

# Files to install ###################################################################

install(TARGETS kio_digikamalbums    DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES digikamalbums.protocol
        DESTINATION ${SERVICES_INSTALL_DIR}
)
