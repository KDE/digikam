#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

include(CTest)
enable_testing()

find_package(Qt5Test REQUIRED)

# test the primitive datatypes and helper functions

set(test_primitives_sources
    test_primitives.cpp
    ../src/groupstatecomputer.cpp
    ../src/kgeomap_common.cpp
    ../src/libkgeomap_debug.cpp
    ../src/backends/mapbackend.cpp
)
add_executable(kgeomap_test_primitives ${test_primitives_sources})
target_link_libraries(kgeomap_test_primitives KF5KGeoMap Qt5::Core Qt5::Test)
add_test(kgeomap_test_primitives ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_primitives)

# test the GeoCoordinates class

set(test_geocoordinates_sources test_geocoordinates.cpp)
add_executable(kgeomap_test_geocoordinates ${test_geocoordinates_sources})
target_link_libraries(kgeomap_test_geocoordinates KF5KGeoMap Qt5::Core Qt5::Test)
add_test(kgeomap_test_geocoordinates ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_geocoordinates)

# test the TileIndex class

set(test_tileindex_sources test_tileindex.cpp)
add_executable(kgeomap_test_tileindex ${test_tileindex_sources})
target_link_libraries(kgeomap_test_tileindex KF5KGeoMap Qt5::Test)
add_test(kgeomap_test_tileindex ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_tileindex)

# test the LookupAltitudeGeonames class

set(test_lookup_altitude_geonames_sources test_lookup_altitude_geonames.cpp)
add_executable(kgeomap_test_lookup_altitude_geonames ${test_lookup_altitude_geonames_sources})
target_link_libraries(kgeomap_test_lookup_altitude_geonames KF5KGeoMap Qt5::Core Qt5::Test)
# do not add this as a test because it only works if there is an internet connection
# add_test(kgeomap_test_lookup_altitude_geonames ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_lookup_altitude_geonames)

# test the marker model

if(NOT WIN32)
    set(test_itemmarkertiler_sources
        test_itemmarkertiler.cpp
        ../src/kgeomap_common.cpp
        ../src/libkgeomap_debug.cpp
    )
    add_executable(kgeomap_test_itemmarkertiler ${test_itemmarkertiler_sources})
    target_link_libraries(kgeomap_test_itemmarkertiler KF5KGeoMap Qt5::Core Qt5::Test KF5::WidgetsAddons)
    add_test(kgeomap_test_itemmarkertiler ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_itemmarkertiler)
endif()

# test the track management classes

set(test_tracks_sources test_tracks.cpp)
add_executable(kgeomap_test_tracks ${test_tracks_sources})
target_link_libraries(kgeomap_test_tracks KF5KGeoMap Qt5::Core Qt5::Test)
add_test(kgeomap_test_tracks ${EXECUTABLE_OUTPUT_PATH}/kgeomap_test_tracks)

# track loading application for timing tests

set(loadtrack_sources loadtrack.cpp)
add_executable(loadtrack ${loadtrack_sources})
target_link_libraries(loadtrack KF5KGeoMap Qt5::Core Qt5::Test)
