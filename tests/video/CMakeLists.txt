#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
)

set(framesencoder_SRCS framesencoder.cpp)
add_executable(framesencoder ${framesencoder_SRCS})
ecm_mark_nongui_executable(framesencoder)

target_link_libraries(framesencoder

                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)

# -------------------------------------------------

set(qtavcodecs_SRCS qtavcodecs.cpp)
add_executable(qtavcodecs ${qtavcodecs_SRCS})
ecm_mark_nongui_executable(qtavcodecs)

target_link_libraries(qtavcodecs

                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)

# -------------------------------------------------

set(player_SRCS player.cpp)
add_executable(player ${player_SRCS})

target_link_libraries(player

                      digikamcore

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)

# -------------------------------------------------

set(videothumbtest_SRCS videothumbtest.cpp)
add_executable(videothumbtest ${videothumbtest_SRCS})
ecm_mark_nongui_executable(videothumbtest)

target_link_libraries(videothumbtest
                      digikamcore

                      Qt5::Core
                      Qt5::Widgets
)
