#
# Copyright (c) 2010-2017 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(pgfscaled_SRCS pgfscaled.cpp ${pgfutils_SRCS})
add_executable(pgfscaled ${pgfscaled_SRCS})
ecm_mark_nongui_executable(pgfscaled)

target_link_libraries(pgfscaled
                      digikamcore
                      libdng

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
)

#------------------------------------------------------------------------

set(qtpgftest_SRCS qtpgftest.cpp ${pgfutils_SRCS})
add_executable(qtpgftest ${qtpgftest_SRCS})
ecm_mark_nongui_executable(qtpgftest)

target_link_libraries(qtpgftest
                      digikamcore
                      libdng

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
)

#------------------------------------------------------------------------

set(loadpgfdata_SRCS loadpgfdata.cpp ${pgfutils_SRCS})
add_executable(loadpgfdata ${loadpgfdata_SRCS})
ecm_mark_nongui_executable(loadpgfdata)

target_link_libraries(loadpgfdata
                      digikamcore
                      libdng

                      Qt5::Core
                      Qt5::Gui

                      KF5::I18n
)

#------------------------------------------------------------------------

set(loadsavethreadtest_SRCS loadsavethreadtest.cpp)
add_executable(loadsavethreadtest ${loadsavethreadtest_SRCS})
ecm_mark_nongui_executable(loadsavethreadtest)

target_link_libraries(loadsavethreadtest
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)

#------------------------------------------------------------------------

set(filesaveoptionsboxtest_SRCS
    filesaveoptionsboxtest.cpp
)

add_executable(filesaveoptionsboxtest ${filesaveoptionsboxtest_SRCS})
add_test(filesaveoptionsboxtest filesaveoptionsboxtest)
ecm_mark_as_test(filesaveoptionsboxtest)

target_link_libraries(filesaveoptionsboxtest
                      digikamcore
                      digikamdatabase

                      libdng

                      Qt5::Test
)

#------------------------------------------------------------------------

set(statesavingobject_SRCS
    statesavingobjecttest.cpp
)

add_executable(statesavingobjecttest ${statesavingobject_SRCS})
add_test(statesavingobjecttest statesavingobjecttest)
ecm_mark_as_test(statesavingobjecttest)

target_link_libraries(statesavingobjecttest
                      digikamcore
                      libdng

                      Qt5::Gui
                      Qt5::Test

                      KF5::XmlGui

                      ${OpenCV_LIBRARIES}
)

if(Gphoto2_FOUND)
    target_link_libraries(statesavingobjecttest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

if(ENABLE_MEDIAPLAYER)

    set(videothumbtest_SRCS videothumbtest.cpp)
    add_executable(videothumbtest ${videothumbtest_SRCS})
    ecm_mark_nongui_executable(videothumbtest)

    target_link_libraries(videothumbtest
                          digikamcore
                          libdng

                          Qt5::Core
                          Qt5::Widgets
    )

endif()

