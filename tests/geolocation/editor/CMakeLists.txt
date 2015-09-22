#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_executable(geolocationedit_test_correlator
               test_correlator.cpp
               ../../../utilities/geolocation/editor/correlator/track_correlator.cpp
               ../../../utilities/geolocation/editor/correlator/track_correlator_thread.cpp
              )

target_link_libraries(geolocationedit_test_correlator
                      digikamcore
                      Qt5::Test
                      Qt5::Gui
                      Qt5::Xml
                      KF5::KIOCore
                      KF5::ItemModels
                     )

# ----------------------------------------------------------------

if(NOT WIN32)

    add_executable(geolocationedit_test_rgparsing
                   test_rgparsing.cpp
                  )

    target_link_libraries(geolocationedit_test_rgparsing
                          digikamcore
                          Qt5::Test
                          Qt5::Gui
                          KF5::ItemModels
                         )

endif()

# ----------------------------------------------------------------

add_executable(geolocationedit_test_rgtagmodel
               test_rgtagmodel.cpp
               ../../../utilities/geolocation/editor/reversegeocoding/rgtagmodel.cpp
               ../../../utilities/geolocation/editor/reversegeocoding/simpletreemodel.cpp
               ../../modeltest/modeltest.cpp
              )

target_link_libraries(geolocationedit_test_rgtagmodel
                      digikamcore
                      Qt5::Test
                      Qt5::Gui
                      KF5::ItemModels
                      KF5::I18n
                     )

# ----------------------------------------------------------------

add_executable(geolocationedit_test_gpsimageitem
               test_gpsimageitem.cpp
               ../../../utilities/geolocation/editor/items/gpsimageitem.cpp
               ../../../utilities/geolocation/editor/items/gpsimagemodel.cpp
              )

target_link_libraries(geolocationedit_test_gpsimageitem
                      digikamcore
                      Qt5::Test
                      Qt5::Gui
                      KF5::ItemModels
                      KF5::KExiv2
                      KF5::KIOCore
                      KF5::I18n
                     )

# ----------------------------------------------------------------
                     
add_executable(geolocationedit_test_simpletreemodel
               test_simpletreemodel.cpp
               ../../../utilities/geolocation/editor/reversegeocoding/simpletreemodel.cpp
               ../../modeltest/modeltest.cpp
              )

target_link_libraries(geolocationedit_test_simpletreemodel
                      Qt5::Test
                      Qt5::Gui
                     )

# ----------------------------------------------------------------

add_test(geolocationedit_test_correlator       ${EXECUTABLE_OUTPUT_PATH}/geolocationedit_test_correlator)
add_test(geolocationedit_test_gpsrgparsing     ${EXECUTABLE_OUTPUT_PATH}/geolocationedit_test_rgparsing)
add_test(geolocationedit_test_gpsrgtagmodel    ${EXECUTABLE_OUTPUT_PATH}/geolocationedit_test_rgtagmodel)
add_test(geolocationedit_test_gpsgpsimageitem  ${EXECUTABLE_OUTPUT_PATH}/geolocationedit_test_gpsimageitem)
add_test(geolocationedit_test_simpletreemodel  ${EXECUTABLE_OUTPUT_PATH}/geolocationedit_test_simpletreemodel)
