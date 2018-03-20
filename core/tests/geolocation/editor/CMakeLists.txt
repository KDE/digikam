#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

add_executable(geolocationedit_test_correlator test_correlator.cpp)
add_test(geolocationedit_test_correlator geolocationedit_test_correlator)
ecm_mark_as_test(geolocationedit_test_correlator)

target_link_libraries(geolocationedit_test_correlator
                      digikamcore

                      Qt5::Test
                      Qt5::Gui
                      Qt5::Xml
                     )

# ----------------------------------------------------------------

if(NOT WIN32)

    add_executable(geolocationedit_test_gpsrgparsing test_rgparsing.cpp)
    add_test(geolocationedit_test_gpsrgparsing geolocationedit_test_gpsrgparsing)
    ecm_mark_as_test(geolocationedit_test_gpsrgparsing)

    target_link_libraries(geolocationedit_test_gpsrgparsing
                          digikamcore

                          Qt5::Test
                          Qt5::Gui
                         )

endif()

# ----------------------------------------------------------------

add_executable(geolocationedit_test_gpsrgtagmodel test_rgtagmodel.cpp ../../modeltest/modeltest.cpp)
add_test(geolocationedit_test_gpsrgtagmodel geolocationedit_test_gpsrgtagmodel)
ecm_mark_as_test(geolocationedit_test_gpsrgtagmodel)

target_link_libraries(geolocationedit_test_gpsrgtagmodel
                      digikamcore

                      Qt5::Test
                      Qt5::Gui

                      KF5::I18n
                     )

# ----------------------------------------------------------------

add_executable(geolocationedit_test_gpsimageitem test_gpsimageitem.cpp)
add_test(geolocationedit_test_gpsimageitem geolocationedit_test_gpsimageitem)
ecm_mark_as_test(geolocationedit_test_gpsimageitem)

target_link_libraries(geolocationedit_test_gpsimageitem
                      digikamcore

                      Qt5::Test
                      Qt5::Gui

                      KF5::I18n
                     )

# ----------------------------------------------------------------

add_executable(geolocationedit_test_simpletreemodel test_simpletreemodel.cpp ../../modeltest/modeltest.cpp)
add_test(geolocationedit_test_simpletreemodel geolocationedit_test_simpletreemodel)
ecm_mark_as_test(geolocationedit_test_simpletreemodel)

target_link_libraries(geolocationedit_test_simpletreemodel
                      digikamcore

                      Qt5::Test
                      Qt5::Gui
                     )
