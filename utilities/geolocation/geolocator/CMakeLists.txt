#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libgeolocator_SRCS backends/backend-rg.cpp
                       backends/backend-geonames-rg.cpp
                       backends/backend-geonamesUS-rg.cpp
                       backends/backend-osm-rg.cpp
                       tracks/track_correlator.cpp
                       tracks/track_correlator_thread.cpp
                       tracks/track_listmodel.cpp
                       reversegeocoding/rgtagmodel.cpp
                       reversegeocoding/rgwidget.cpp
                       searches/searchbackend.cpp
                       searches/searchwidget.cpp
                       mapdragdrophandler.cpp
                       kipiimagemodel.cpp
                       kipiimageitem.cpp
                       kipiimagelist.cpp
                       gpssetup.cpp
                       gpssetupgeneral.cpp
                       gpsimagelistdragdrophandler.cpp
                       gpslistviewcontextmenu.cpp
                       gpscorrelatorwidget.cpp
                       gpsimagedetails.cpp
                       gpsundocommand.cpp
                       gpsbookmarkowner.cpp
                       gpssyncdialog.cpp
                       tests/simpletreemodel/simpletreemodel.cpp
   )

add_library(geolocator_src OBJECT ${libgeolocator_SRCS})

add_library(geolocator STATIC $<TARGET_OBJECTS:geolocator_src>)

target_link_libraries(geolocator
                      PRIVATE
                      Qt5::Gui
                      Qt5::Concurrent

                      KF5::KExiv2
                      KF5::KDcraw
                      KF5::KGeoMap
                      KF5::ItemModels
                      KF5::IconThemes
                      KF5::GuiAddons
                      KF5::Bookmarks
                      KF5::KHtml
                    )
