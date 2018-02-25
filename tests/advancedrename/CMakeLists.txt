#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(renamecustomizertest_SRCS
    renamecustomizertest.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
)

add_executable(renamecustomizertest ${renamecustomizertest_SRCS})
add_test(renamecustomizertest renamecustomizertest)
ecm_mark_as_test(renamecustomizertest)

target_link_libraries(renamecustomizertest

                      digikamcore
                      digikamgui

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(cameranamehelper_SRCS
    cameranamehelpertest.cpp
)

add_executable(cameranamehelpertest ${cameranamehelper_SRCS})
add_test(cameranamehelpertest cameranamehelpertest)
ecm_mark_as_test(cameranamehelpertest)

target_link_libraries(cameranamehelpertest

                      digikamdatabase
                      digikamcore
                      digikamgui

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)

if(Gphoto2_FOUND)
    target_link_libraries(cameranamehelpertest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(advancedrenametest_SRCS
    advancedrenametest.cpp
)

add_executable(advancedrenametest ${advancedrenametest_SRCS})
add_test(advancedrenametest advancedrenametest)
ecm_mark_as_test(advancedrenametest)

target_link_libraries(advancedrenametest

                      digikamdatabase
                      digikamcore
                      digikamgui

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::Solid
                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)
