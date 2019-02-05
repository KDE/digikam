#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(calibrator_sources calibrator.cpp)

add_executable(geoiface_calibrator ${calibrator_sources})

target_link_libraries(geoiface_calibrator
                      digikamcore

                      Qt5::Widgets

                      KF5::I18n
                     )
