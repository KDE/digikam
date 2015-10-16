#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(
    $<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::JobWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Notifications,INTERFACE_INCLUDE_DIRECTORIES>
)

set(testdatabase_SRCS testdatabase.cpp)
add_executable(testdatabase ${testdatabase_SRCS})

target_link_libraries(testdatabase

                      digikamgui

                      Qt5::DBus
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test
                      Qt5::Sql

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KIOCore
                      KF5::KExiv2
                      KF5::JobWidgets
                      KF5::Notifications

                      ${OpenCV_LIBRARIES}
)

if(${KF5Kipi_FOUND})
    target_link_libraries(testdatabase  KF5::Kipi)
endif()

#------------------------------------------------------------------------

set(databasefieldstest_srcs databasefieldstest.cpp)
add_executable(databasefieldstest ${databasefieldstest_srcs})
add_test(databasefieldstest databasefieldstest)
ecm_mark_as_test(databasefieldstest)

target_link_libraries(databasefieldstest

                      digikamgui

                      Qt5::DBus
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test
                      Qt5::Sql

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KIOCore
                      KF5::KExiv2
                      KF5::JobWidgets
                      KF5::Notifications
)
