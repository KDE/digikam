#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(pgfscaled_SRCS pgfscaled.cpp ${pgfutils_SRCS})
add_executable(pgfscaled ${pgfscaled_SRCS})
ecm_mark_nongui_executable(pgfscaled)

target_link_libraries(pgfscaled
                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      KF5::KIOCore
)

if(MARBLE_FOUND)
    target_link_libraries(pgfscaled ${MARBLE_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(qtpgftest_SRCS qtpgftest.cpp ${pgfutils_SRCS})
add_executable(qtpgftest ${qtpgftest_SRCS})
ecm_mark_nongui_executable(qtpgftest)

target_link_libraries(qtpgftest
                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      KF5::KIOCore
)

if(MARBLE_FOUND)
    target_link_libraries(qtpgftest ${MARBLE_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(loadpgfdata_SRCS loadpgfdata.cpp ${pgfutils_SRCS})
add_executable(loadpgfdata ${loadpgfdata_SRCS})
ecm_mark_nongui_executable(loadpgfdata)

target_link_libraries(loadpgfdata
                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      KF5::KIOCore
)

if(MARBLE_FOUND)
    target_link_libraries(loadpgfdata ${MARBLE_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(loadsavethreadtest_SRCS loadsavethreadtest.cpp)
add_executable(loadsavethreadtest ${loadsavethreadtest_SRCS})
ecm_mark_nongui_executable(loadsavethreadtest)

target_link_libraries(loadsavethreadtest

                      digikamcore

                      Qt5::Core

                      KF5::I18n
                      KF5::KExiv2
                      KF5::KDcraw
)

if(MARBLE_FOUND)
    target_link_libraries(loadsavethreadtest ${MARBLE_LIBRARIES})
endif()

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

                       Qt5::Test
 )

if(MARBLE_FOUND)
    target_link_libraries(filesaveoptionsboxtest ${MARBLE_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(statesavingobject_SRCS
    statesavingobjecttest.cpp
)

add_executable(statesavingobjecttest ${statesavingobject_SRCS})
add_test(statesavingobjecttest statesavingobjecttest)
ecm_mark_as_test(statesavingobjecttest)

target_link_libraries(statesavingobjecttest
                      digikamcore
                      Qt5::Gui
                      Qt5::Test
                      KF5::XmlGui
                      ${OpenCV_LIBRARIES}
)

if(GPHOTO2_FOUND)
    target_link_libraries(statesavingobjecttest ${GPHOTO2_LIBRARIES})
endif()

if(MARBLE_FOUND)
    target_link_libraries(statesavingobjecttest ${MARBLE_LIBRARIES})
endif()
