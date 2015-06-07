#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libkgeomaphelper_SRCS digikam2kgeomap.cpp)

add_library(kgeomaphelper STATIC ${libkgeomaphelper_SRCS})

target_link_libraries(kgeomaphelper
                      PRIVATE
                      KF5::KGeoMap
)
