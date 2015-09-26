#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(calibrator_sources calibrator.cpp)

add_executable(geoiface_calibrator ${calibrator_sources})

target_link_libraries(geoiface_calibrator
                      PUBLIC
                          digikamcore
                          Qt5::Widgets
                      PRIVATE
                          KF5::I18n
                     )
