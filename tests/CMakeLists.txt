#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# enabling for everything although only required by DImg.
kde_enable_exceptions()

add_subdirectory(albummodel)
add_subdirectory(advancedrename)
add_subdirectory(dimg)
add_subdirectory(dmetadata)
add_subdirectory(geolocation)
add_subdirectory(imgqsort)
add_subdirectory(iojobs)
add_subdirectory(timestampupdate)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

if(ENABLE_DIGIKAM_MODELTEST)
    message(STATUS "Modeltest enabled")
    set(libdigikamgui_SRCS ${libdigikamgui_SRCS}
        modeltest/modeltest.cpp
       )
    add_definitions(-DENABLE_DIGIKAM_MODELTEST)
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

set(searchtextbartest_SRCS
    searchtextbartest.cpp
)

add_executable(searchtextbartest ${searchtextbartest_SRCS})
add_test(searchtextbartest searchtextbartest)
ecm_mark_as_test(searchtextbartest)

target_link_libraries(searchtextbartest

                      digikamcore
                      KF5::Kipi  # about dialog requires kipi
                      KF5::Completion
                      Qt5::Test
)

if(MARBLE_FOUND)
    target_link_libraries(searchtextbartest ${MARBLE_LIBRARIES})
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

#------------------------------------------------------------------------

set(testautocrop_SRCS testautocrop.cpp)
add_executable(testautocrop ${testautocrop_SRCS})
ecm_mark_nongui_executable(testautocrop)

target_link_libraries(testautocrop

                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::XmlGui

                      KF5::KExiv2
                      KF5::KDcraw

                      ${OpenCV_LIBRARIES}
)

if(MARBLE_FOUND)
    target_link_libraries(testautocrop ${MARBLE_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(testdatabase_SRCS testdatabase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/album/album.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/album/albumhistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/album/albummanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/album/albumwatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database/core/scancontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database/core/databaseguierrorhandler.cpp
   )
add_executable(testdatabase ${testdatabase_SRCS}
                            $<TARGET_OBJECTS:digikamsettings_src>
                            $<TARGET_OBJECTS:notificationmanager_src>
                            $<TARGET_OBJECTS:setup_src>
)

target_link_libraries(testdatabase

                      digikamgui
                      Qt5::DBus
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test
                      Qt5::Sql

                      KF5::I18n
                      KF5::XmlGui
                      KF5::KIOCore
                      KF5::KExiv2
                      KF5::KDcraw
                      KF5::JobWidgets
                      KF5::Notifications

                      ${OpenCV_LIBRARIES}
)
if(${KF5Kipi_FOUND})
    target_link_libraries(testdatabase  KF5::Kipi)
endif()

#------------------------------------------------------------------------

if(LENSFUN_FOUND)

    set(testlensfuniface_SRCS testlensfuniface.cpp)
    add_executable(testlensfuniface ${testlensfuniface_SRCS})
    ecm_mark_nongui_executable(testlensfuniface)

    target_link_libraries(testlensfuniface

                      digikamcore

                      Qt5::Core

                      KF5::I18n
                      KF5::KIOCore
                      KF5::KExiv2

                      ${LENSFUN_LIBRARIES}
                      ${OpenCV_LIBRARIES}
    )

    if(NOT WIN32)
        # To link under Solaris (see bug #274484)
        target_link_libraries(testlensfuniface ${MATH_LIBRARY})
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
        target_link_libraries(testlensfuniface ${KVM_LIBRARY})
    endif()

    if(MARBLE_FOUND)
        target_link_libraries(testlensfuniface ${MARBLE_LIBRARIES})
    endif()

endif()

#------------------------------------------------------------------------

set(testnrestimate_SRCS testnrestimate.cpp)
add_executable (testnrestimate ${testnrestimate_SRCS})
ecm_mark_nongui_executable(testnrestimate)

target_link_libraries(testnrestimate

                      digikamcore
                      digikamdatabase
                      digikamcore

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::KIOCore
                      KF5::KExiv2
                      KF5::KDcraw

                      ${OpenCV_LIBRARIES}
)

if(MARBLE_FOUND)
    target_link_libraries(testnrestimate ${MARBLE_LIBRARIES})
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

# NOTE: code ported to Qt5 but do not link yet.
# undefined reference to `Digikam::DatabaseFields::FieldMetaInfo<QFlags<Digikam::DatabaseFields::ImagesField> >::Last'
# happens even though file defining those libs/database/framework/databasefields.h is included and linking against digikamdatabase.
#
# set(databasefieldstest_SRCS
#     databasefieldstest.cpp
# )

# add_executable(databasefieldstest ${databasefieldstest_SRCS})
# add_test(databasefieldstest databasefieldstest)
# ecm_mark_as_test(databasefieldstest)

# target_link_libraries(databasefieldstest

#                       digikamdatabase
#                       Qt5::Test
#                       Qt5::Core

#                       KF5::I18n
#                       KF5::KExiv2
# )
