#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_executable(gpssync_test_correlator
               test_correlator.cpp
               ../correlator/track_correlator.cpp
               ../correlator/track_correlator_thread.cpp
              )

target_link_libraries(gpssync_test_correlator
                      Qt5::Test
                      Qt5::Gui
                      Qt5::Xml
                      KF5::KIOCore
                      KF5::ItemModels
                      KF5::KGeoMap
                     )

# ----------------------------------------------------------------

if(NOT WIN32)

    add_executable(gpssync_test_rgparsing
                   test_rgparsing.cpp
                  )

    target_link_libraries(gpssync_test_rgparsing
                          Qt5::Test
                          Qt5::Gui
                          KF5::ItemModels
                          KF5::KGeoMap
                         )

endif()

# ----------------------------------------------------------------

add_executable(gpssync_test_rgtagmodel
               test_rgtagmodel.cpp
               ../reversegeocoding/rgtagmodel.cpp
               ../reversegeocoding/simpletreemodel.cpp
               ../../../../tests/modeltest/modeltest.cpp
              )

target_link_libraries(gpssync_test_rgtagmodel
                      Qt5::Test
                      Qt5::Gui
                      KF5::ItemModels
                      KF5::KGeoMap
                      KF5::I18n
                     )

# ----------------------------------------------------------------

add_executable(gpssync_test_gpsimageitem
               test_gpsimageitem.cpp
               ../items/gpsimageitem.cpp
               ../items/gpsimagemodel.cpp
              )

target_link_libraries(gpssync_test_gpsimageitem
                      digikamcore
                      Qt5::Test
                      Qt5::Gui
                      KF5::ItemModels
                      KF5::KExiv2
                      KF5::KGeoMap
                      KF5::KIOCore
                      KF5::I18n
                     )

# ----------------------------------------------------------------
                     
add_executable(gpssync_test_simpletreemodel
               test_simpletreemodel.cpp
               ../reversegeocoding/simpletreemodel.cpp
               ../../../../tests/modeltest/modeltest.cpp
              )

target_link_libraries(gpssync_test_simpletreemodel
                      Qt5::Test
                      Qt5::Gui
                     )

# ----------------------------------------------------------------

add_test(gpssync_test_correlator       ${EXECUTABLE_OUTPUT_PATH}/gpssync_test_correlator)
add_test(gpssync_test_gpsrgparsing     ${EXECUTABLE_OUTPUT_PATH}/gpssync_test_rgparsing)
add_test(gpssync_test_gpsrgtagmodel    ${EXECUTABLE_OUTPUT_PATH}/gpssync_test_rgtagmodel)
add_test(gpssync_test_gpsgpsimageitem ${EXECUTABLE_OUTPUT_PATH}/gpssync_test_gpsimageitem)
add_test(gpssync_test_simpletreemodel  ${EXECUTABLE_OUTPUT_PATH}/gpssync_test_simpletreemodel)
