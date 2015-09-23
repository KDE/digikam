#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(albummodeltest_SRCS
    albummodeltest.cpp
    ../modeltest/modeltest.cpp
)

add_executable(albummodeltest ${albummodeltest_SRCS})
add_test(albummodeltest albummodeltest)
ecm_mark_as_test(albummodeltest)

target_link_libraries(albummodeltest

                      digikamgui

                      Qt5::Gui
                      Qt5::Test
                      Qt5::Sql
                      Qt5::Xml

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KExiv2
                      KF5::KDcraw
                      KF5::KIOCore

                      ${OpenCV_LIBRARIES}
)