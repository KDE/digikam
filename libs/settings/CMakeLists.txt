#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libsettings_SRCS
    applicationsettings.cpp
    applicationsettings_p.cpp
    applicationsettings_albums.cpp
    applicationsettings_database.cpp
    applicationsettings_iconview.cpp
    applicationsettings_mime.cpp
    applicationsettings_miscs.cpp
    applicationsettings_tooltips.cpp
)

add_library(digikamsettings STATIC ${libsettings_SRCS})


target_link_libraries(digikamsettings

                      digikamwidgets #required for DMultiTabbar 

                      Qt5::Gui
                      Qt5::Widgets
)
