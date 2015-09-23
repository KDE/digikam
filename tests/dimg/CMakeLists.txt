#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(dimgfilteractiontest_SRCS
    dimgfilteractiontest.cpp
)

add_executable(dimgfilteractiontest ${dimgfilteractiontest_SRCS})
add_test(dimgfilteractiontest dimgfilteractiontest)
ecm_mark_as_test(dimgfilteractiontest)

target_link_libraries(dimgfilteractiontest

                      digikamcore

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test

                      KF5::KIOCore
                      KF5::XmlGui
                      KF5::I18n
                      KF5::KExiv2

                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(dimghistorygraphtest_SRCS
    dimgabstracthistorytest.cpp
    dimghistorygraphtest.cpp
    ../modeltest/modeltest.cpp
)

add_executable(dimghistorygraphtest ${dimghistorygraphtest_SRCS})
add_test(dimghistorygraphtest dimghistorygraphtest)
ecm_mark_as_test(dimghistorygraphtest)

target_link_libraries(dimghistorygraphtest

                      digikamcore
                      digikamdatabase

                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KExiv2
                      KF5::KIOWidgets # for previewjob

                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(dimghistorytest_SRCS
    dimgabstracthistorytest.cpp
    dimghistorytest.cpp
)

add_executable(dimghistorytest ${dimghistorytest_SRCS})
add_test(dimghistorytest dimghistorytest)
ecm_mark_as_test(dimghistorytest)

target_link_libraries(dimghistorytest

                      digikamcore
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KExiv2
                      KF5::KIOWidgets # for previewjob
                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(dimgfreerotationtest_SRCS
    dimgfreerotationtest.cpp
)

add_executable(dimgfreerotationtest ${dimgfreerotationtest_SRCS})
add_test(dimgfreerotationtest dimgfreerotationtest)
ecm_mark_as_test(dimgfreerotationtest)

target_link_libraries(dimgfreerotationtest

                      digikamcore

                      Qt5::Test
)

#------------------------------------------------------------------------

set(testdimgloader_SRCS testdimgloader.cpp)
add_executable(testdimgloader ${testdimgloader_SRCS})
ecm_mark_nongui_executable(testdimgloader)

target_link_libraries(testdimgloader

                      digikamcore
                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KExiv2
                      KF5::KDcraw

                      ${OpenCV_LIBRARIES}
)
