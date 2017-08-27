#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libgeolocationedit_SRCS backends/backend-rg.cpp
                            backends/backend-geonames-rg.cpp
                            backends/backend-geonamesUS-rg.cpp
                            backends/backend-osm-rg.cpp

                            correlator/track_correlator.cpp
                            correlator/track_correlator_thread.cpp
                            correlator/track_listmodel.cpp
                            correlator/gpscorrelatorwidget.cpp

                            reversegeocoding/rginfo.cpp
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
                            dialog/geolocationedit.cpp
                            dialog/gpsgeoifacemodelhelper.cpp

                            bookmark/gpsbookmarkowner.cpp
                            bookmark/gpsbookmarkmodelhelper.cpp
                            bookmark/bookmarknode.cpp
                            bookmark/bookmarksmenu.cpp
                            bookmark/bookmarksmngr.cpp
                            bookmark/bookmarksdlg.cpp

                            kmlexport/geodataparser.cpp
                            kmlexport/kmlgpsdataparser.cpp
                            kmlexport/kmlwidget.cpp
                            kmlexport/kmlexport.cpp

)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5Bookmarks_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::Bookmarks,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_library(geolocationedit_src OBJECT ${libgeolocationedit_SRCS})

add_library(geolocationedit STATIC $<TARGET_OBJECTS:geolocationedit_src>)

target_link_libraries(geolocationedit
                      PRIVATE
                      Qt5::Gui
                      Qt5::Concurrent
                    )
