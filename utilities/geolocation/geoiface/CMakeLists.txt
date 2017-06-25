#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libgeoiface_SRCS backends/mapbackend.cpp
                     backends/backendmarble.cpp
                     backends/backendmarblelayer.cpp
                     backends/backendgooglemaps.cpp
                     tracks/trackreader.cpp
                     tracks/trackmanager.cpp
                     lookup/lookupaltitude.cpp
                     lookup/lookupaltitudegeonames.cpp
                     lookup/lookupfactory.cpp
                     core/geodragdrophandler.cpp
                     core/geocoordinates.cpp
                     core/geoifacecommon.cpp
                     core/geomodelhelper.cpp
                     core/groupstatecomputer.cpp
                     tiles/abstractmarkertiler.cpp
                     tiles/itemmarkertiler.cpp
                     tiles/tilegrouper.cpp
                     tiles/tileindex.cpp
                     widgets/htmlwidget.cpp
                     widgets/mapwidget.cpp
                     widgets/placeholderwidget.cpp
)

include_directories($<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>

                    ${MARBLE_INCLUDE_DIR}
)

# Marble translations need explicit loading
ecm_create_qm_loader(libgeoiface_SRCS marble_qt)

add_library(geoiface_src OBJECT ${libgeoiface_SRCS})
