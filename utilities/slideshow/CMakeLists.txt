#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libslideshow_SRCS
    slidetoolbar.cpp
    slideosd.cpp
    slideproperties.cpp
    slideimage.cpp
    slideerror.cpp
    slideend.cpp
    slideshow.cpp
    slidehelp.cpp
    slideshowsettings.cpp
)

add_library(slideshow STATIC ${libslideshow_SRCS})


target_link_libraries(slideshow
                      PRIVATE
                      digikamwidgets
                      digikamdatabase_static
                      digikamalbum


                      Qt5::Widgets

                      KF5::WidgetsAddons
                      KF5::KDcraw
                      KF5::KExiv2
)
