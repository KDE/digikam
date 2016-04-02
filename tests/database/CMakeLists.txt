#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5Notifications_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::Notifications,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

set(testdatabase_SRCS testdatabase.cpp)
add_executable(testdatabase ${testdatabase_SRCS})

target_link_libraries(testdatabase
                      digikamcore
                      digikamgui

                      libdng

                      Qt5::DBus
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test
                      Qt5::Sql

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KIOCore

                      ${OpenCV_LIBRARIES}
)

if(KF5Notifications_FOUND)
    target_link_libraries(testdatabase PUBLIC KF5::Notifications)
endif()

if(KF5Kipi_FOUND)
    target_link_libraries(testdatabase KF5::Kipi)
endif()

#------------------------------------------------------------------------

set(databasefieldstest_srcs databasefieldstest.cpp)
add_executable(databasefieldstest ${databasefieldstest_srcs})
add_test(databasefieldstest databasefieldstest)
ecm_mark_as_test(databasefieldstest)

target_link_libraries(databasefieldstest

                      digikamgui

                      libdng

                      Qt5::DBus
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test
                      Qt5::Sql

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KIOCore
)

if(KF5Notifications_FOUND)
    target_link_libraries(databasefieldstest PUBLIC KF5::Notifications)
endif()

#------------------------------------------------------------------------

# set(databasetagstest_srcs databasetagstest.cpp)
# add_executable(databasetagstest ${databasetagstest_srcs})
# add_test(databasetagstest databasetagstest)
# ecm_mark_as_test(databasetagstest)
#
# target_link_libraries(databasetagstest
#
#                       digikamgui
#
#                       libdng
#
#                       Qt5::DBus
#                       Qt5::Core
#                       Qt5::Gui
#                       Qt5::Test
#                       Qt5::Sql
#
#                       KF5::I18n
#                       KF5::XmlGui
#                       KF5::KIOCore
#)
#
#if(KF5Notifications_FOUND)
#    target_link_libraries(databasetagstest PUBLIC KF5::Notifications)
#endif()
