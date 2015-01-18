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
target_link_libraries(lighttable
                      digikamcore
                      #Qt5::Core
                      #Qt5::Gui
                      #Qt5::Sql
                      #Qt5::Widgets
                      
                      #KF5::KDELibs4Support # for KMultiTabBar
                      #KF5::NotifyConfig
)

install( FILES lighttablewindowui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam )
