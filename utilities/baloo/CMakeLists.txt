#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(baloowrap_SRCS baloowrap.cpp)

add_library(baloowrap STATIC ${baloowrap_SRCS})

target_link_libraries(baloowrap

                      KF5::Baloo
                      KF5::FileMetaData
)

