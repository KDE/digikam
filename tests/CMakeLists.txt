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
add_subdirectory(fileio)
add_subdirectory(filters)
add_subdirectory(timestampupdate)

if(ENABLE_DIGIKAM_MODELTEST)
    message(STATUS "Modeltest enabled")
    set(libdigikamgui_SRCS ${libdigikamgui_SRCS}
        modeltest/modeltest.cpp
       )
    add_definitions(-DENABLE_DIGIKAM_MODELTEST)
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
