#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libgpssearch_SRCS
    gpssearchview.cpp
    gpsmarkertiler.cpp
)

add_library(gpssearch_src OBJECT ${libgpssearch_SRCS})
