#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(imgqsort)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#------------------------------------------------------------------------

set(advancedrenametest_SRCS
    advancedrenametest.cpp
)

add_executable(advancedrenametest ${advancedrenametest_SRCS})
add_test(advancedrenametest advancedrenametest)
ecm_mark_as_test(advancedrenametest)

target_link_libraries(advancedrenametest
                      Qt5::Gui
                      Qt5::Test
                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
                      KF5::KDELibs4Support
                      ${OpenCV_LIBRARIES}
                      digikamcore
                      digikamdatabase
                      advancedrename
)

#------------------------------------------------------------------------

set(renamecustomizertest_SRCS
    renamecustomizertest.cpp
)

add_executable(renamecustomizertest ${renamecustomizertest_SRCS})
add_test(renamecustomizertest renamecustomizertest)
ecm_mark_as_test(renamecustomizertest)

target_link_libraries(renamecustomizertest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      digikamcore
                      digikamdatabase
                      importui
                      advancedrename
                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(cameranamehelper_SRCS
    cameranamehelpertest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/importui/views/cameranamehelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/importui/backend/gpcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/importui/backend/camiteminfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/importui/backend/dkcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_albums.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_iconview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_mime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_miscs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_tooltips.cpp
)

add_executable(cameranamehelpertest ${cameranamehelper_SRCS})
add_test(cameranamehelpertest cameranamehelpertest)
ecm_mark_as_test(cameranamehelpertest)

target_link_libraries(cameranamehelpertest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      digikamcore
                      digikamdatabase
                      ${KF5KDcraw_LIBRARIES}
                      ${KF5KExiv2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
)

if(GPHOTO2_FOUND)
    target_link_libraries(cameranamehelpertest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(uifilevalidator_SRCS
    uifilevalidatortest.cpp
)

add_executable(uifilevalidatortest ${uifilevalidator_SRCS})
add_test(uifilevalidatortest uifilevalidatortest)
ecm_mark_as_test(uifilevalidatortest)

target_link_libraries(uifilevalidatortest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      digikamcore
                      digikamdatabase
                      ${OpenCV_LIBRARIES}
)

if(GPHOTO2_FOUND)
    target_link_libraries(uifilevalidatortest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(statesavingobject_SRCS
    statesavingobjecttest.cpp
)

add_executable(statesavingobjecttest ${statesavingobject_SRCS})
add_test(statesavingobjecttest statesavingobjecttest)
ecm_mark_as_test(statesavingobjecttest)

target_link_libraries(statesavingobjecttest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      digikamcore
                      ${OpenCV_LIBRARIES}
)

if(GPHOTO2_FOUND)
    target_link_libraries(statesavingobjecttest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(freerotationtest_SRCS
    freerotationtest.cpp
)

add_executable(freerotationtest ${freerotationtest_SRCS})
add_test(freerotationtest freerotationtest)
ecm_mark_as_test(freerotationtest)

target_link_libraries(freerotationtest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      digikamcore
                      ${OpenCV_LIBRARIES}
                     )

#------------------------------------------------------------------------

set(searchtextbartest_SRCS
    searchtextbartest.cpp
)

add_executable(searchtextbartest ${searchtextbartest_SRCS})
add_test(searchtextbartest searchtextbartest)
ecm_mark_as_test(searchtextbartest)

target_link_libraries(searchtextbartest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      digikamcore
                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(filesaveoptionsboxtest_SRCS
    filesaveoptionsboxtest.cpp
)

add_executable(filesaveoptionsboxtest ${filesaveoptionsboxtest_SRCS})
add_test(filesaveoptionsboxtest filesaveoptionsboxtest)
ecm_mark_as_test(filesaveoptionsboxtest)

target_link_libraries(filesaveoptionsboxtest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      digikamcore
                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

#set(albummodeltest_SRCS
#    albummodeltest.cpp
#    modeltest/modeltest.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../app/album/album.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../app/album/albummanager.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../app/album/albumsettings.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../app/album/albumthumbnailloader.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../app/database/databaseguierrorhandler.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/common/databasewidget.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../app/database/scancontroller.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/importui/dkcamera.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/importui/gpcamera.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/cameralist.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/cameraselection.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/cameratype.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setup.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupdatabase.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupcollections.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupcollectionview.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupalbumview.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupcategory.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setuptooltip.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupmetadata.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setuptemplate.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupmime.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setuplighttable.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupeditor.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupiofiles.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupdcraw.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupicc.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupplugins.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupslideshow.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupscriptmanager.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/scriptplugintype.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupversioning.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupcamera.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupmisc.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupversioning.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/batch/batchthumbsgenerator.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/models/abstractalbummodel.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/models/albumfiltermodel.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/models/albummodel.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/template/subjectedit.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/template/templatelist.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/template/templatemanager.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/template/templatepanel.cpp
#   )
#
#add_executable(albummodeltest ${albummodeltest_SRCS})
add_test(albummodeltest albummodeltest)
ecm_mark_as_test(albummodeltest)
#
#target_link_libraries(albummodeltest
#                      Qt5::Gui
#                      Qt5::Test
#                      Qt5::Xml
#                      Qt5::Sql
#                      KF5::KIOCore
#                      KF5::KDELibs4Support
#                      ${KF5KDcraw_LIBRARIES}
#                      ${KF5KExiv2_LIBRARIES}
#                      digikamdatabase
#                      digikamcore
#                     )
#
#if(${KF5Kipi_FOUND})
#    target_link_libraries(albummodeltest KF5::Kipi)
#endif()
#
#if(GPHOTO2_FOUND)
#     target_link_libraries(albummodeltest ${GPHOTO2_LIBRARIES})
#endif()

#------------------------------------------------------------------------

set(dimagehistorytest_SRCS
    abstractdimagehistorytest.cpp
    dimagehistorytest.cpp
)

add_executable(dimagehistorytest ${dimagehistorytest_SRCS})
add_test(dimagehistorytest dimagehistorytest)
ecm_mark_as_test(dimagehistorytest)

target_link_libraries(dimagehistorytest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      ${KF5KExiv2_LIBRARIES}
                      digikamcore
                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(dimagehistorygraphtest_SRCS
    abstractdimagehistorytest.cpp
    dimagehistorygraphtest.cpp
    modeltest/modeltest.cpp
)

add_executable(dimagehistorygraphtest ${dimagehistorygraphtest_SRCS})
add_test(dimagehistorygraphtest dimagehistorygraphtest)
ecm_mark_as_test(dimagehistorygraphtest)

target_link_libraries(dimagehistorygraphtest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      KF5::KIOCore
                      ${KF5KExiv2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamdatabase
                      digikamcore
)

#------------------------------------------------------------------------

set(dimagefilteractiontest_SRCS
    dimagefilteractiontest.cpp
)

add_executable(dimagefilteractiontest ${dimagefilteractiontest_SRCS})
add_test(dimagefilteractiontest dimagefilteractiontest)
ecm_mark_as_test(dimagefilteractiontest)

target_link_libraries(dimagefilteractiontest
                      Qt5::Gui
                      Qt5::Test
                      KF5::KDELibs4Support
                      ${KF5KExiv2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamcore
)

#------------------------------------------------------------------------

set(databasefieldstest_SRCS
    databasefieldstest.cpp
)

#add_executable(databasefieldstest ${databasefieldstest_SRCS})
add_test(databasefieldstest databasefieldstest)
ecm_mark_as_test(databasefieldstest)

#target_link_libraries(databasefieldstest
#                      Qt5::Gui
#                      Qt5::Test
#                      KF5::KDELibs4Support
#                      ${KF5KExiv2_LIBRARIES}
#                      digikamcore
#)

#------------------------------------------------------------------------

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/threadimageio
                    ${PGF_INCLUDE_DIRS}
)

set(qtpgftest_SRCS qtpgftest.cpp ${pgfutils_SRCS})
add_executable(qtpgftest ${qtpgftest_SRCS})
ecm_mark_nongui_executable(qtpgftest)
target_link_libraries(qtpgftest
                      Qt5::Core
                      Qt5::Gui
                      KF5::KIOCore
                      KF5::KDELibs4Support
                      ${PGF_LIBRARIES}
)

set(pgfscaled_SRCS pgfscaled.cpp ${pgfutils_SRCS})
add_executable(pgfscaled ${pgfscaled_SRCS})
ecm_mark_nongui_executable(pgfscaled)
target_link_libraries(pgfscaled
                      Qt5::Core
                      Qt5::Gui
                      KF5::KIOCore
                      KF5::KDELibs4Support
                      ${PGF_LIBRARIES}
)

set(loadpgfdata_SRCS loadpgfdata.cpp ${pgfutils_SRCS})
add_executable(loadpgfdata ${loadpgfdata_SRCS})
ecm_mark_nongui_executable(loadpgfdata)
target_link_libraries(loadpgfdata
                      Qt5::Core
                      Qt5::Gui
                      KF5::KIOCore
                      KF5::KDELibs4Support
                      ${PGF_LIBRARIES}
)

#------------------------------------------------------------------------

set(testdimgloader_SRCS testdimgloader.cpp)
add_executable(testdimgloader ${testdimgloader_SRCS})
ecm_mark_nongui_executable(testdimgloader)
target_link_libraries(testdimgloader
                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      KF5::KDELibs4Support
                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
                      ${OpenCV_LIBRARIES}
)

set(loadsavethreadtest_SRCS loadsavethreadtest.cpp)
add_executable(loadsavethreadtest ${loadsavethreadtest_SRCS})
ecm_mark_nongui_executable(loadsavethreadtest)
target_link_libraries(loadsavethreadtest
                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      KF5::KDELibs4Support
                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
                      ${OpenCV_LIBRARIES}
)

if(LENSFUN_FOUND)

    set(testlensfuniface_SRCS testlensfuniface.cpp)
    add_executable(testlensfuniface ${testlensfuniface_SRCS})
    ecm_mark_nongui_executable(testlensfuniface)
    target_link_libraries(testlensfuniface
                          digikamcore
                          Qt5::Core
                          KF5::KDELibs4Support
                          ${KF5KExiv2_LIBRARIES}
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

endif()

#------------------------------------------------------------------------

set (testnrestimate_SRCS testnrestimate.cpp)
add_executable (testnrestimate ${testnrestimate_SRCS})
ecm_mark_nongui_executable(testnrestimate)
target_link_libraries(testnrestimate
                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      KF5::KDELibs4Support
                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(testautocrop_SRCS testautocrop.cpp)
add_executable(testautocrop ${testautocrop_SRCS})
ecm_mark_nongui_executable(testautocrop)
target_link_libraries(testautocrop
                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      KF5::KDELibs4Support
                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
                      ${OpenCV_LIBRARIES}
)

#------------------------------------------------------------------------

set(testdatabase_SRCS testdatabase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/album/album.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/album/albumhistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/album/albummanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/album/albumwatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/database/scancontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/database/databaseguierrorhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_albums.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_iconview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_miscs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_mime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../app/settings/applicationsettings_tooltips.cpp
)

add_executable(testdatabase ${testdatabase_SRCS})
target_link_libraries(testdatabase
                      digikamcore
                      digikamdatabase
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql
                      KF5::KDELibs4Support
                      KF5::KIOCore
                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
                      ${OpenCV_LIBRARIES}
)

if(ENABLE_DIGIKAM_MODELTEST)
    message(STATUS "Modeltest enabled")
    set(libdigikamgui_SRCS ${libdigikamgui_SRCS}
        modeltest/modeltest.cpp
       )
    add_definitions(-DENABLE_DIGIKAM_MODELTEST)
endif()