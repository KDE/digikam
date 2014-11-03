ADD_SUBDIRECTORY(imgqsort)

SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

#------------------------------------------------------------------------

SET(advancedrenametest_SRCS
    advancedrenametest.cpp
   )

KDE4_ADD_UNIT_TEST(advancedrenametest ${advancedrenametest_SRCS})

TARGET_LINK_LIBRARIES(advancedrenametest
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

SET(renamecustomizertest_SRCS
    renamecustomizertest.cpp
   )

KDE4_ADD_UNIT_TEST(renamecustomizertest ${renamecustomizertest_SRCS})

TARGET_LINK_LIBRARIES(renamecustomizertest
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

SET(cameranamehelper_SRCS
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

TARGET_LINK_LIBRARIES(cameranamehelpertest
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${KDCRAW_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                      digikamdatabase
                     )

IF(GPHOTO2_FOUND)
    TARGET_LINK_LIBRARIES(cameranamehelpertest ${GPHOTO2_LIBRARIES})
ENDIF()

#------------------------------------------------------------------------

SET(uifilevalidator_SRCS
    uifilevalidatortest.cpp
   )

KDE4_ADD_UNIT_TEST(uifilevalidatortest ${uifilevalidator_SRCS})

TARGET_LINK_LIBRARIES(uifilevalidatortest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                      digikamdatabase
                     )

IF(GPHOTO2_FOUND)
    TARGET_LINK_LIBRARIES(uifilevalidatortest ${GPHOTO2_LIBRARIES})
ENDIF()

#------------------------------------------------------------------------

SET(statesavingobject_SRCS
    statesavingobjecttest.cpp
   )

KDE4_ADD_UNIT_TEST(statesavingobjecttest ${statesavingobject_SRCS})

TARGET_LINK_LIBRARIES(statesavingobjecttest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

IF(GPHOTO2_FOUND)
    TARGET_LINK_LIBRARIES(statesavingobjecttest ${GPHOTO2_LIBRARIES})
ENDIF()

#------------------------------------------------------------------------

SET(freerotationtest_SRCS
    freerotationtest.cpp
   )
KDE4_ADD_UNIT_TEST(freerotationtest ${freerotationtest_SRCS})

TARGET_LINK_LIBRARIES(freerotationtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

SET(searchtextbartest_SRCS
    searchtextbartest.cpp
   )

KDE4_ADD_UNIT_TEST(searchtextbartest ${searchtextbartest_SRCS})

TARGET_LINK_LIBRARIES(searchtextbartest
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

SET(filesaveoptionsboxtest_SRCS
    filesaveoptionsboxtest.cpp
   )

KDE4_ADD_UNIT_TEST(filesaveoptionsboxtest ${filesaveoptionsboxtest_SRCS})

TARGET_LINK_LIBRARIES(filesaveoptionsboxtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

#SET(albummodeltest_SRCS
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
#TARGET_LINK_LIBRARIES(albummodeltest
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
#IF(KIPI_FOUND)
#    TARGET_LINK_LIBRARIES(albummodeltest ${KIPI_LIBRARIES})
#ENDIF()
#
#IF(GPHOTO2_FOUND)
#     TARGET_LINK_LIBRARIES(albummodeltest ${GPHOTO2_LIBRARIES})
#ENDIF()

#------------------------------------------------------------------------

SET(dimagehistorytest_SRCS
    abstractdimagehistorytest.cpp
    dimagehistorytest.cpp
   )

KDE4_ADD_UNIT_TEST(dimagehistorytest ${dimagehistorytest_SRCS})

TARGET_LINK_LIBRARIES(dimagehistorytest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${KEXIV2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

SET(dimagehistorygraphtest_SRCS
    abstractdimagehistorytest.cpp
    dimagehistorygraphtest.cpp
    modeltest/modeltest.cpp
   )

KDE4_ADD_UNIT_TEST(dimagehistorygraphtest ${dimagehistorygraphtest_SRCS})

TARGET_LINK_LIBRARIES(dimagehistorygraphtest
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

SET(dimagefilteractiontest_SRCS
    dimagefilteractiontest.cpp
   )

KDE4_ADD_UNIT_TEST(dimagefilteractiontest ${dimagefilteractiontest_SRCS})

TARGET_LINK_LIBRARIES(dimagefilteractiontest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${KEXIV2_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                      digikamcore
                     )

#------------------------------------------------------------------------

SET(databasefieldstest_SRCS
    databasefieldstest.cpp
   )

#KDE4_ADD_UNIT_TEST(databasefieldstest ${databasefieldstest_SRCS})

#TARGET_LINK_LIBRARIES(databasefieldstest
#                      ${KDE4_KDECORE_LIBS}
#                      ${QT_QTGUI_LIBRARY}
#                      ${QT_QTTEST_LIBRARY}
#                      ${KEXIV2_LIBRARIES}
#                      digikamcore
#                     )

#------------------------------------------------------------------------

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../libs/threadimageio)

INCLUDE_DIRECTORIES(${PGF_INCLUDE_DIRS})

SET(qtpgftest_SRCS qtpgftest.cpp ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(qtpgftest NOGUI ${qtpgftest_SRCS})
TARGET_LINK_LIBRARIES(qtpgftest ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${PGF_LIBRARIES})

SET(pgfscaled_SRCS pgfscaled.cpp ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(pgfscaled NOGUI ${pgfscaled_SRCS})
TARGET_LINK_LIBRARIES(pgfscaled ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${PGF_LIBRARIES})

SET(loadpgfdata_SRCS loadpgfdata.cpp ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(loadpgfdata NOGUI ${loadpgfdata_SRCS})
TARGET_LINK_LIBRARIES(loadpgfdata ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${PGF_LIBRARIES})

#------------------------------------------------------------------------

SET(testdimgloader_SRCS testdimgloader.cpp)
KDE4_ADD_EXECUTABLE(testdimgloader NOGUI ${testdimgloader_SRCS})
TARGET_LINK_LIBRARIES(testdimgloader
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )

SET(loadsavethreadtest_SRCS loadsavethreadtest.cpp)
KDE4_ADD_EXECUTABLE(loadsavethreadtest NOGUI ${loadsavethreadtest_SRCS})
TARGET_LINK_LIBRARIES(loadsavethreadtest
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )

IF(LENSFUN_FOUND)

    SET(testlensfuniface_SRCS testlensfuniface.cpp)
    KDE4_ADD_EXECUTABLE(testlensfuniface NOGUI ${testlensfuniface_SRCS})
    TARGET_LINK_LIBRARIES(testlensfuniface
                          digikamcore
                          ${QT_QTCORE_LIBRARY}
                          ${KDE4_KDECORE_LIBS}
                          ${KEXIV2_LIBRARIES}

                          ${LENSFUN_LIBRARIES}
                          ${OpenCV_LIBRARIES}
                         )

    IF(NOT WIN32)
        # To link under Solaris (see bug #274484)
        TARGET_LINK_LIBRARIES(testlensfuniface ${MATH_LIBRARY})
    ENDIF()

    IF(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
        TARGET_LINK_LIBRARIES(testlensfuniface ${KVM_LIBRARY})
    ENDIF()

ENDIF()

#------------------------------------------------------------------------

SET (testnrestimate_SRCS testnrestimate.cpp)
KDE4_ADD_EXECUTABLE (testnrestimate NOGUI ${testnrestimate_SRCS})
TARGET_LINK_LIBRARIES(testnrestimate
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )

#------------------------------------------------------------------------

SET(testautocrop_SRCS testautocrop.cpp)
KDE4_ADD_EXECUTABLE(testautocrop NOGUI ${testautocrop_SRCS})
TARGET_LINK_LIBRARIES(testautocrop
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )

#------------------------------------------------------------------------

SET(testdatabase_SRCS testdatabase.cpp
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
TARGET_LINK_LIBRARIES(testdatabase
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
