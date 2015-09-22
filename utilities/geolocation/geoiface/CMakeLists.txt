#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libgeoiface_SRCS backends/mapbackend.cpp
                     backends/htmlwidget.cpp
                     backends/backendmarble.cpp
                     backends/backendmarblelayer.cpp
                     backends/backendgooglemaps.cpp

                     abstractmarkertiler.cpp
                     dragdrophandler.cpp
                     geocoordinates.cpp
                     groupstatecomputer.cpp
                     itemmarkertiler.cpp
                     geoiface_common.cpp
                     geoiface_debug.cpp
                     lookupaltitude.cpp
                     lookupaltitudegeonames.cpp
                     lookupfactory.cpp
                     mapwidget.cpp
                     modelhelper.cpp
                     placeholderwidget.cpp
                     tilegrouper.cpp
                     tileindex.cpp
                     trackreader.cpp
                     tracks.cpp
)

add_library(geoiface_src OBJECT ${libgeoiface_SRCS})
