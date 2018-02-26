#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

# Boost uses operator names (and, not, ...)
string(REPLACE "-fno-operator-names" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include_directories(
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
)

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

                      KF5::XmlGui
                      KF5::I18n

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

                      Qt5::Sql
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::XmlGui

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

                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(testequalizefilter_SRCS testequalizefilter.cpp)
add_executable(testequalizefilter ${testequalizefilter_SRCS})
ecm_mark_nongui_executable(testequalizefilter)

target_link_libraries(testequalizefilter

                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(testcolorbalancefilter_SRCS testcolorbalancefilter.cpp)
add_executable(testcolorbalancefilter ${testcolorbalancefilter_SRCS})
ecm_mark_nongui_executable(testcolorbalancefilter)

target_link_libraries(testcolorbalancefilter

                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)
