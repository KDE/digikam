#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

set(timestampupdatetest_SRCS timestampupdatetest.cpp)
add_executable(timestampupdatetest ${timestampupdatetest_SRCS})
add_test(timestampupdatetest timestampupdatetest)
ecm_mark_as_test(timestampupdatetest)

target_link_libraries(timestampupdatetest
                      digikamdatabase
                      digikamcore

                      Qt5::Gui
                      Qt5::Test
                      Qt5::Sql

                      KF5::ConfigCore
                      KF5::I18n
                     )

if(ENABLE_DBUS)
    target_link_libraries(timestampupdatetest Qt5::DBus)
endif()
