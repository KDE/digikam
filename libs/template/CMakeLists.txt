#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libtemplate_SRCS
    templatemanager.cpp
    templatelist.cpp
    templateselector.cpp
    templateviewer.cpp
    templatepanel.cpp
    subjectedit.cpp
)


add_library(digikamtemplate_src OBJECT  ${libtemplate_SRCS})
add_library(digikamtemplate STATIC $<TARGET_OBJECTS:digikamtemplate_src>)

target_link_libraries(digikamtemplate
                      PRIVATE
                      dmetadata

                      Qt5::Core
                      Qt5::Widgets
                      Qt5::Xml

                      KF5::WidgetsAddons
)
