#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

add_subdirectory(calibrator)
add_subdirectory(demo)

# -- test the primitive datatypes and helper functions -----------------------------------------

set(test_primitives_sources
    test_primitives.cpp)

add_executable(geoiface_test_primitives ${test_primitives_sources})
add_test(geoiface_test_primitives geoiface_test_primitives)
ecm_mark_as_test(geoiface_test_primitives)

target_link_libraries(geoiface_test_primitives
                      digikamcore

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test
                     )

# -- test the GeoCoordinates class --------------------------------------------------------------

set(test_geocoordinates_sources test_geocoordinates.cpp)

add_executable(geoiface_test_geocoordinates ${test_geocoordinates_sources})
add_test(geoiface_test_geocoordinates geoiface_test_geocoordinates)
ecm_mark_as_test(geoiface_test_geocoordinates)

target_link_libraries(geoiface_test_geocoordinates
                      digikamcore

                      Qt5::Core
                      Qt5::Test
                     )

# -- test the TileIndex class --------------------------------------------------------------------

set(test_tileindex_sources test_tileindex.cpp)

add_executable(geoiface_test_tileindex ${test_tileindex_sources})
add_test(geoiface_test_tileindex geoiface_test_tileindex)
ecm_mark_as_test(geoiface_test_tileindex)

target_link_libraries(geoiface_test_tileindex
                      digikamcore

                      Qt5::Test
                     )

# -- test the LookupAltitudeGeonames class -------------------------------------------------------

# do not add this as a test because it only works if there is an internet connection
#
# set(test_lookup_altitude_geonames_sources test_lookup_altitude_geonames.cpp)
#
# add_executable(geoiface_test_lookup_altitude_geonames ${test_lookup_altitude_geonames_sources})
# add_test(geoiface_test_lookup_altitude_geonames geoiface_test_lookup_altitude_geonames)
# ecm_mark_as_test(geoiface_test_lookup_altitude_geonames)
#
# target_link_libraries(geoiface_test_lookup_altitude_geonames
#                       digikamcore
#                       Qt5::Core
#                       Qt5::Test)

# -- test the marker model ------------------------------------------------------------------------

if(NOT WIN32)

    set(test_itemmarkertiler_sources
        test_itemmarkertiler.cpp)

    add_executable(geoiface_test_itemmarkertiler ${test_itemmarkertiler_sources})
    add_test(geoiface_test_itemmarkertiler geoiface_test_itemmarkertiler)
    ecm_mark_as_test(geoiface_test_itemmarkertiler)

    target_link_libraries(geoiface_test_itemmarkertiler
                          digikamcore

                          Qt5::Core
                          Qt5::Gui
                          Qt5::Test
                         )

endif()

# -- test the track management classes -------------------------------------------------------------

set(test_tracks_sources test_tracks.cpp)

add_executable(geoiface_test_tracks ${test_tracks_sources})
add_test(geoiface_test_tracks geoiface_test_tracks)
ecm_mark_as_test(geoiface_test_tracks)

target_link_libraries(geoiface_test_tracks
                      digikamcore

                      Qt5::Core
                      Qt5::Test
                     )

# -- track loading application for timing tests ----------------------------------------------------

set(loadtrack_sources loadtrack.cpp)

add_executable(loadtrack ${loadtrack_sources})

target_link_libraries(loadtrack
                      digikamcore

                      Qt5::Core
                     )
