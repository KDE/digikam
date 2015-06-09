#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(liblighttable_SRCS
    lighttablepreview.cpp
    lighttableview.cpp
    lighttablethumbbar.cpp
    lighttablewindow.cpp
)

add_library(lighttable_src OBJECT ${liblighttable_SRCS})

add_library(lighttable STATIC $<TARGET_OBJECTS:lighttable_src>)

target_link_libraries(lighttable
                      PRIVATE
                      digikamdatabase
                      imagepropertiesdigikam
                      imageeditor

                      Qt5::Gui
)

install( FILES lighttablewindowui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam )
