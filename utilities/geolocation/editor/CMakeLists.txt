#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libgeolocationedit_SRCS backends/backend-rg.cpp
                            backends/backend-geonames-rg.cpp
                            backends/backend-geonamesUS-rg.cpp
                            backends/backend-osm-rg.cpp

                            correlator/track_correlator.cpp
                            correlator/track_correlator_thread.cpp
                            correlator/track_listmodel.cpp
                            correlator/gpscorrelatorwidget.cpp

                            reversegeocoding/rgtagmodel.cpp
                            reversegeocoding/rgwidget.cpp
                            reversegeocoding/simpletreemodel.cpp

                            searches/searchbackend.cpp
                            searches/searchresultmodel.cpp
                            searches/searchresultmodelhelper.cpp
                            searches/searchwidget.cpp

                            dragdrop/mapdragdrophandler.cpp
                            dragdrop/gpsimagelistdragdrophandler.cpp

                            items/gpsimagemodel.cpp
                            items/gpsimagesortproxymodel.cpp
                            items/gpsimageitem.cpp
                            items/gpsimageitemdelegate.cpp
                            items/gpsimagelist.cpp
                            items/gpsimagelistcontextmenu.cpp

                            dialog/gpscommon.cpp
                            dialog/gpsimagedetails.cpp
                            dialog/gpsundocommand.cpp
                            dialog/gpsbookmarkowner.cpp
                            dialog/gpsbookmarkmodelhelper.cpp
                            dialog/gpssyncdialog.cpp
                            dialog/gpssynckgeomapmodelhelper.cpp
   )

add_library(geolocationedit_src OBJECT ${libgeolocationedit_SRCS})

add_library(geolocationedit STATIC $<TARGET_OBJECTS:geolocationedit_src>)

target_link_libraries(geolocationedit
                      PRIVATE
                      Qt5::Gui
                      Qt5::Concurrent

                      KF5::KExiv2
                      KF5::KDcraw
                    )

if(BUILD_TESTING)
    ecm_optional_add_subdirectory(tests)
endif()
