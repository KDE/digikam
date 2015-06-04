#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libfirstrun_SRCS
    assistantdlg.cpp
    assistantdlgpage.cpp
    welcomepage.cpp
    collectionpage.cpp
    rawpage.cpp
    tooltipspage.cpp
    previewpage.cpp
    openfilepage.cpp
    metadatapage.cpp
    startscanpage.cpp
)

add_library(firstrun STATIC ${libfirstrun_SRCS})

target_include_directories(firstrun
                           PRIVATE
                           "$<TARGET_PROPERTY:digikamwidgets,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>"
                           "$<TARGET_PROPERTY:digikamsettings,INTERFACE_INCLUDE_DIRECTORIES>"
)

target_link_libraries(firstrun
                      PRIVATE
                      digikamdatabase

                      Qt5::Widgets
                      
                      KF5::WidgetsAddons
                      KF5::ConfigWidgets
                      KF5::KIOWidgets
                      KF5::Kipi # kxmlgui dependecy

                      KF5::KExiv2
                      KF5::KDcraw
)
