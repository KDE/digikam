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

add_library(lighttable STATIC ${liblighttable_SRCS})

target_include_directories(lighttable
                           PRIVATE
                           ${DK_LOCAL_INCLUDES}
                           ${KF5_INCLUDES_ALL}
)
target_link_libraries(lighttable

                      digikamdatabase

                      Qt5::Gui
)

install( FILES lighttablewindowui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam )
