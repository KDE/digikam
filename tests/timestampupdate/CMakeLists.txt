#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
)

set(timestampupdatetest_SRCS timestampupdatetest.cpp)
add_executable(timestampupdatetest ${timestampupdatetest_SRCS})
add_test(timestampupdatetest timestampupdatetest)
ecm_mark_as_test(timestampupdatetest)

target_link_libraries(timestampupdatetest

                      digikamdatabase
                      digikamcore

                      Qt5::Gui
                      Qt5::Test
                      Qt5::DBus

                      KF5::I18n
                      KF5::KExiv2
                     )
