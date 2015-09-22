#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/../)

set(calibrator_sources
    calibrator.cpp
)

add_executable(libkgeomap_calibrator ${calibrator_sources})

target_link_libraries(libkgeomap_calibrator
    PUBLIC
        Qt5::Widgets
        KF5KGeoMap
    PRIVATE
        KF5::CoreAddons
        KF5::I18n
)
