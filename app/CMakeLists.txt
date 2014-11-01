CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/utils/version.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/utils/version.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/database/dbconfigversion.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/database/dbconfigversion.h)

# Shared libdigikamcore ################################################################

# Disable liblqr C code warnings.
SET_SOURCE_FILES_PROPERTIES(${liblqr_SRCS} PROPERTIES COMPILE_FLAGS "-w")

# Disable liblenfun C code warnings.
SET_SOURCE_FILES_PROPERTIES(${liblensfun_SRCS} PROPERTIES COMPILE_FLAGS "-w")

SET(digikamcore_LIB_SRCS
        # basic libs
        ${libdimg_SRCS}
        ${libdimgloaders_SRCS}
        ${libdimgfilters_SRCS}
        ${libdmetadata_SRCS}
        ${libjpegutils_SRCS}
        ${libprogressmanager_SRCS}
        ${libdatabasecore_SRCS}
        ${libthreadimageio_SRCS}
        ${pgfutils_SRCS}
        ${libdigikamhelpers_SRCS}
        ${libtthread_SRCS}
        ${libversionmanager_SRCS}
        ${libkde3rdparty_SRCS}

        # widgets and dialogs
        ${libcommonwidgets_SRCS}
        ${libnotificationmanager_SRCS}
        ${libmainview_SRCS}
        ${libitemview_SRCS}
        ${libeditorwidgets_SRCS}
        ${libiccprofileswidgets_SRCS}
        ${libmetadatawidgets_SRCS}
        ${libdialogs_SRCS}
        ${libimageproperties_SRCS}
        ${libgraphicsviewwidgets_SRCS}
        ${libgenericmodels_SRCS}

        # utilities
        ${libslideshow_SRCS}
        ${libeditorcore_SRCS}
        ${libeditordlg_SRCS}
        ${libeditorprintiface_SRCS}
        ${libeditorrawimport_SRCS}
        ${libeditoriface_SRCS}
        ${libeditorplugin_SRCS}
)

IF(KGEOMAP_FOUND)
    SET(digikamcore_LIB_SRCS ${digikamcore_LIB_SRCS}
                             ${libkgeomaphelper_SRCS}
       )
ENDIF(KGEOMAP_FOUND)

KDE4_ADD_UI_FILES(digikamcore_LIB_SRCS
                  ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/printiface/printoptionspage.ui
                 )

KDE4_ADD_KCFG_FILES(digikamcore_LIB_SRCS
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/printiface/digikamconfig.kcfgc
                   )

KDE4_ADD_LIBRARY(digikamcore SHARED ${digikamcore_LIB_SRCS})

ADD_DEPENDENCIES(digikamcore digikam-gitversion)

TARGET_LINK_LIBRARIES(digikamcore
        ${QT_QTSQL_LIBRARY}

        ${KDE4_KDECORE_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4_KHTML_LIBS}
        ${KDE4_KNOTIFYCONFIG_LIBRARY}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KUTILS_LIBS}
        ${KDE4_SOLID_LIBS}
        ${KDE4_THREADWEAVER_LIBRARIES}

        ${JPEG_LIBRARIES}
        ${TIFF_LIBRARIES}
        ${PNG_LIBRARIES}
        ${LCMS_LIBRARIES}
        ${OpenCV_LIBRARIES}
        ${PGF_LIBRARIES}

        # Required by CImg library which is using pthread internally.
        ${CMAKE_THREAD_LIBS_INIT}

        ${KDCRAW_LIBRARIES}
        ${KEXIV2_LIBRARIES}
        ${LCMS_LIBRARIES}
)

IF(Q_WS_X11)
    TARGET_LINK_LIBRARIES(digikamcore ${X11_LIBRARIES})
ENDIF(Q_WS_X11)

IF(APPLE)
    TARGET_LINK_LIBRARIES(digikamcore /System/Library/Frameworks/AppKit.framework)
ENDIF(APPLE)

# For Contents Aware Resizer and Lens auto-correction tools.
IF(GLIB2_FOUND)
    TARGET_LINK_LIBRARIES(digikamcore ${GLIB2_LIBRARIES})
ENDIF(GLIB2_FOUND)

IF(JASPER_FOUND)
    TARGET_LINK_LIBRARIES(digikamcore ${JASPER_LIBRARIES})
ENDIF(JASPER_FOUND)

IF(LENSFUN_FOUND)
   TARGET_LINK_LIBRARIES(digikamcore ${LENSFUN_LIBRARIES})
ENDIF(LENSFUN_FOUND)

IF(LQR-1_FOUND)
   TARGET_LINK_LIBRARIES(digikamcore ${liblqr_LIBS})
ENDIF(LQR-1_FOUND)

IF(KIPI_FOUND)
   TARGET_LINK_LIBRARIES(digikamcore ${KIPI_LIBRARIES})
ENDIF(KIPI_FOUND)

IF(KFACE_FOUND)
   TARGET_LINK_LIBRARIES(digikamcore ${KFACE_LIBRARIES})
ENDIF(KFACE_FOUND)

IF(KGEOMAP_FOUND)
   TARGET_LINK_LIBRARIES(digikamcore ${KGEOMAP_LIBRARIES})
ENDIF(KGEOMAP_FOUND)

SET_TARGET_PROPERTIES(digikamcore PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

IF(WIN32)
    SET_TARGET_PROPERTIES(digikamcore PROPERTIES COMPILE_FLAGS -DJPEG_STATIC)
    SET_TARGET_PROPERTIES(digikamcore PROPERTIES OUTPUT_NAME digikamlib)
ENDIF(WIN32)

INSTALL(TARGETS digikamcore ${INSTALL_TARGETS_DEFAULT_ARGS})

# Shared libdigikamdatabase ########################################################

SET(digikamdatabase_LIB_SRCS
        ${libdatabase_SRCS}
        ${libdatabasemodels_SRCS}
        ${libhaar_SRCS}
        ${pgfutils_SRCS}
)

IF(Baloo_FOUND)
    SET(digikamdatabase_LIB_SRCS ${digikamdatabase_LIB_SRCS} ${libbaloowrap_SRCS})
ENDIF(Baloo_FOUND)

IF(SQLITE2_FOUND)
    SET(digikamdatabase_LIB_SRCS ${digikamdatabase_LIB_SRCS} ${libalbumsqlite2_SRCS})
ENDIF(SQLITE2_FOUND)

KDE4_ADD_LIBRARY(digikamdatabase SHARED ${digikamdatabase_LIB_SRCS})

TARGET_LINK_LIBRARIES(digikamdatabase
                      digikamcore
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTSQL_LIBRARY}

                      ${KDE4_KIO_LIBS}
                      ${KDE4_SOLID_LIBS}

                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}

                      ${PGF_LIBRARIES}
                     )

IF(KFACE_FOUND)
   TARGET_LINK_LIBRARIES(digikamdatabase ${KFACE_LIBRARIES})
ENDIF(KFACE_FOUND)

IF(KGEOMAP_FOUND)
   TARGET_LINK_LIBRARIES(digikamdatabase ${KGEOMAP_LIBRARIES})
ENDIF(KGEOMAP_FOUND)

IF(Baloo_FOUND)
    TARGET_LINK_LIBRARIES(digikamdatabase ${BALOO_LIBRARIES})
ENDIF(Baloo_FOUND)

IF(SQLITE2_FOUND)
    TARGET_LINK_LIBRARIES(digikamdatabase ${SQLITE2_LIBRARIES})
ENDIF(SQLITE2_FOUND)

SET_TARGET_PROPERTIES(digikamdatabase PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

INSTALL(TARGETS digikamdatabase ${INSTALL_TARGETS_DEFAULT_ARGS} )


# digiKam binary progam #########################################################

SET(digikam_SRCS ${libeditorgui_SRCS}
                 ${libdeletedialog_SRCS}
                 ${libimagepropertiesdigikam_SRCS}
                 ${libimageversionssidebar_SRCS}
                 ${libsetup_SRCS}
                 ${libimportui_SRCS}
                 ${liblighttable_SRCS}
                 ${libmaintenance_SRCS}
                 ${libsearchwindow_SRCS}
                 ${libfuzzysearch_SRCS}
                 ${libqueuemanager_SRCS}
                 ${libalbummodels_SRCS}
                 ${libdigikamgui_SRCS}
                 ${libfirstrun_SRCS}
                 ${libtemplate_SRCS}
                 ${libdatabaseui_SRCS}
                 ${libimgqsort_SRCS}
                 main/main.cpp
)

IF(KIPI_FOUND)
    SET(digikam_SRCS ${digikam_SRCS} ${libkipiiface_SRCS})
ENDIF(KIPI_FOUND)

IF(KFACE_FOUND)
    SET(digikam_SRCS ${digikam_SRCS} ${libfacemanagement_SRCS})
ENDIF(KFACE_FOUND)

IF(KGEOMAP_FOUND)
    SET(digikam_SRCS ${digikam_SRCS} ${libkgeomaphelper_SRCS} ${libgpssearch_SRCS})
ENDIF(KGEOMAP_FOUND)

QT4_ADD_DBUS_ADAPTOR(digikam_SRCS main/org.kde.digikam.xml main/digikamapp.h Digikam::DigikamApp)

KDE4_ADD_APP_ICON(digikam_SRCS "../data/icons/apps/hi*-app-digikam.png")

KDE4_ADD_EXECUTABLE(digikam ${digikam_SRCS})

# TO fill plist XML file for OSX #####################################################

SET(MACOSX_APP_NAME_STRING             "digikam")
SET(MACOSX_APP_DESCRIPTION             "Advanced digital photo management application")
SET(MACOSX_BUNDLE_LONG_VERSION_STRING  ${DIGIKAM_VERSION_STRING})
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING ${DIGIKAM_VERSION_SHORT})
SET(MACOSX_BUNDLE_BUNDLE_VERSION       ${DIGIKAM_VERSION_STRING})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/Info.plist.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
SET_TARGET_PROPERTIES(digikam PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)


TARGET_LINK_LIBRARIES(digikam
        ${QT_QTSQL_LIBRARY}
        ${QT_QTTEST_LIBRARY}

        ${KDE4_KPARTS_LIBS}
        ${KDE4_PHONON_LIBS}
        ${KDE4_KHTML_LIBS}
        ${KDE4_KUTILS_LIBS}
        ${KDE4_KNOTIFYCONFIG_LIBRARY}
        ${KDE4_SOLID_LIBS}
        ${KDE4_THREADWEAVER_LIBRARIES}

        ${KEXIV2_LIBRARIES}
        ${KDCRAW_LIBRARIES}

        ${STRIGI_LIBRARIES}
        ${LCMS_LIBRARIES}

        digikamcore
        digikamdatabase
        advancedrename
        queuemanager

        importui

        ${OpenCV_LIBRARIES}
)

IF(KDEPIMLIBS_FOUND)
  TARGET_LINK_LIBRARIES(digikam ${KDE4_KABC_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_CONTACT_LIBS})
ENDIF(KDEPIMLIBS_FOUND)

IF(KIPI_FOUND)
   TARGET_LINK_LIBRARIES(digikam ${KIPI_LIBRARIES})
ENDIF(KIPI_FOUND)

IF(KFACE_FOUND)
   TARGET_LINK_LIBRARIES(digikam ${KFACE_LIBRARIES})
ENDIF(KFACE_FOUND)

IF(KGEOMAP_FOUND)
   TARGET_LINK_LIBRARIES(digikam ${KGEOMAP_LIBRARIES})
ENDIF(KGEOMAP_FOUND)

IF(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    TARGET_LINK_LIBRARIES(digikam ${KVM_LIBRARY})
ENDIF(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)

# See bug #258931: libgphoto2 library must be the last arg for linker.

IF(GPHOTO2_FOUND)
    TARGET_LINK_LIBRARIES(digikam ${GPHOTO2_LIBRARIES})
ENDIF(GPHOTO2_FOUND)

IF(NOT WIN32 AND GPHOTO2_FOUND)
    # See bug #268267 : digiKam need to be linked to libusb to prevent crash at gphoto2 init if opencv is linked with libdc1394
    TARGET_LINK_LIBRARIES(digikam ${LIBUSB_LIBRARIES})
ENDIF(NOT WIN32 AND GPHOTO2_FOUND)

INSTALL(TARGETS digikam ${INSTALL_TARGETS_DEFAULT_ARGS})

# Others Files to install #######################################################

INSTALL(PROGRAMS main/digikam.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL(FILES main/digikam.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)
INSTALL(FILES main/digikamui.rc        DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES main/digikam.notifyrc    DESTINATION ${DATA_INSTALL_DIR}/digikam)
