#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(imgqsort)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

#------------------------------------------------------------------------

set(advancedrenametest_SRCS
    advancedrenametest.cpp
   )

KDE4_ADD_UNIT_TEST(advancedrenametest ${advancedrenametest_SRCS})

target_link_libraries(advancedrenametest
                      ${KDE4_KDEUI_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                      digikamdatabase
                      advancedrename
                     )

#------------------------------------------------------------------------

set(renamecustomizertest_SRCS
    renamecustomizertest.cpp
   )

KDE4_ADD_UNIT_TEST(renamecustomizertest ${renamecustomizertest_SRCS})

target_link_libraries(renamecustomizertest
                      ${KDE4_KDEUI_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                      digikamdatabase
                      importui
                      advancedrename
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

KDE4_ADD_UNIT_TEST(cameranamehelpertest ${cameranamehelper_SRCS})

target_link_libraries(cameranamehelpertest
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${KDCRAW_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                      digikamdatabase
                     )

if(GPHOTO2_FOUND)
    target_link_libraries(cameranamehelpertest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(uifilevalidator_SRCS
    uifilevalidatortest.cpp
   )

KDE4_ADD_UNIT_TEST(uifilevalidatortest ${uifilevalidator_SRCS})

target_link_libraries(uifilevalidatortest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                      digikamdatabase
                     )

if(GPHOTO2_FOUND)
    target_link_libraries(uifilevalidatortest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(timestampupdate_SRCS
    timestampupdatetest.cpp
   )

KDE4_ADD_UNIT_TEST(timestampupdatetest ${timestampupdate_SRCS})

target_link_libraries(timestampupdatetest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${KEXIV2_LIBRARIES}
                      digikamcore
                      digikamdatabase
                     )

#------------------------------------------------------------------------

set(statesavingobject_SRCS
    statesavingobjecttest.cpp
   )

KDE4_ADD_UNIT_TEST(statesavingobjecttest ${statesavingobject_SRCS})

target_link_libraries(statesavingobjecttest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

if(GPHOTO2_FOUND)
    target_link_libraries(statesavingobjecttest ${GPHOTO2_LIBRARIES})
endif()

#------------------------------------------------------------------------

set(freerotationtest_SRCS
    freerotationtest.cpp
   )

KDE4_ADD_UNIT_TEST(freerotationtest ${freerotationtest_SRCS})

target_link_libraries(freerotationtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

set(searchtextbartest_SRCS
    searchtextbartest.cpp
   )

KDE4_ADD_UNIT_TEST(searchtextbartest ${searchtextbartest_SRCS})

target_link_libraries(searchtextbartest
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

set(filesaveoptionsboxtest_SRCS
    filesaveoptionsboxtest.cpp
   )

KDE4_ADD_UNIT_TEST(filesaveoptionsboxtest ${filesaveoptionsboxtest_SRCS})

target_link_libraries(filesaveoptionsboxtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
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
#KDE4_ADD_UNIT_TEST(albummodeltest ${albummodeltest_SRCS})
#
#target_link_libraries(albummodeltest
#                      ${KDE4_KIO_LIBS}
#                      ${QT_QTGUI_LIBRARY}
#                      ${QT_QTTEST_LIBRARY}
#                      ${QT_QTXML_LIBRARY}
#                      ${QT_QTSQL_LIBRARY}
#                      ${KDCRAW_LIBRARIES}
#                      ${KEXIV2_LIBRARIES}
#                      digikamdatabase
#                      digikamcore
#                     )
#
#if(KIPI_FOUND)
#    target_link_libraries(albummodeltest ${KIPI_LIBRARIES})
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

KDE4_ADD_UNIT_TEST(dimagehistorytest ${dimagehistorytest_SRCS})

target_link_libraries(dimagehistorytest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${KEXIV2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

set(dimagehistorygraphtest_SRCS
    abstractdimagehistorytest.cpp
    dimagehistorygraphtest.cpp
    modeltest/modeltest.cpp
   )

KDE4_ADD_UNIT_TEST(dimagehistorygraphtest ${dimagehistorygraphtest_SRCS})

target_link_libraries(dimagehistorygraphtest
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${KEXIV2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamdatabase
                      digikamcore
                     )

#------------------------------------------------------------------------

set(dimagefilteractiontest_SRCS
    dimagefilteractiontest.cpp
   )

KDE4_ADD_UNIT_TEST(dimagefilteractiontest ${dimagefilteractiontest_SRCS})

target_link_libraries(dimagefilteractiontest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${KEXIV2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

set(databasefieldstest_SRCS
    databasefieldstest.cpp
   )

#KDE4_ADD_UNIT_TEST(databasefieldstest ${databasefieldstest_SRCS})

#target_link_libraries(databasefieldstest
#                      ${KDE4_KDECORE_LIBS}
#                      ${QT_QTGUI_LIBRARY}
#                      ${QT_QTTEST_LIBRARY}
#                      ${KEXIV2_LIBRARIES}
#                      digikamcore
#                     )

#------------------------------------------------------------------------

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/threadimageio
                    ${PGF_INCLUDE_DIRS}
                   )

set(qtpgftest_SRCS qtpgftest.cpp ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(qtpgftest NOGUI ${qtpgftest_SRCS})
target_link_libraries(qtpgftest ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${PGF_LIBRARIES})

set(pgfscaled_SRCS pgfscaled.cpp ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(pgfscaled NOGUI ${pgfscaled_SRCS})
target_link_libraries(pgfscaled ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${PGF_LIBRARIES})

set(loadpgfdata_SRCS loadpgfdata.cpp ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(loadpgfdata NOGUI ${loadpgfdata_SRCS})
target_link_libraries(loadpgfdata ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${PGF_LIBRARIES})

#------------------------------------------------------------------------

set(testdimgloader_SRCS testdimgloader.cpp)
KDE4_ADD_EXECUTABLE(testdimgloader NOGUI ${testdimgloader_SRCS})
target_link_libraries(testdimgloader
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )

set(loadsavethreadtest_SRCS loadsavethreadtest.cpp)
KDE4_ADD_EXECUTABLE(loadsavethreadtest NOGUI ${loadsavethreadtest_SRCS})
target_link_libraries(loadsavethreadtest
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )

if(LENSFUN_FOUND)

    set(testlensfuniface_SRCS testlensfuniface.cpp)
    KDE4_ADD_EXECUTABLE(testlensfuniface NOGUI ${testlensfuniface_SRCS})
    target_link_libraries(testlensfuniface
                          digikamcore
                          ${QT_QTCORE_LIBRARY}
                          ${KDE4_KDECORE_LIBS}
                          ${KEXIV2_LIBRARIES}

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
KDE4_ADD_EXECUTABLE (testnrestimate NOGUI ${testnrestimate_SRCS})
target_link_libraries(testnrestimate
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )

#------------------------------------------------------------------------

set(testautocrop_SRCS testautocrop.cpp)
KDE4_ADD_EXECUTABLE(testautocrop NOGUI ${testautocrop_SRCS})
target_link_libraries(testautocrop
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
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

KDE4_ADD_EXECUTABLE(testdatabase ${testdatabase_SRCS})
target_link_libraries(testdatabase
                      digikamcore
                      digikamdatabase
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTSQL_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )
