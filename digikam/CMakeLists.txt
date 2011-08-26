CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/utils/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/utils/version.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/../data/database/dbconfigversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/database/dbconfigversion.h)

# Shared libdigikamcore ################################################################

IF(NOT CLAPACK_FOUND)
   # Disable Clapack C code warnings.
   SET_SOURCE_FILES_PROPERTIES(${libclapack_SRCS} PROPERTIES COMPILE_FLAGS "-w")
ENDIF(NOT CLAPACK_FOUND)

IF(NOT PGF_FOUND)
   # Disable libpgf warnings.
   SET_SOURCE_FILES_PROPERTIES(${libpgf_SRCS} PROPERTIES COMPILE_FLAGS "-w")
   IF(OPENMP_FOUND)
        SET_SOURCE_FILES_PROPERTIES(${libpgf_SRCS} PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
   ENDIF(OPENMP_FOUND)
ENDIF(NOT PGF_FOUND)

# Disable liblqr C code warnings.
SET_SOURCE_FILES_PROPERTIES(${liblqr_SRCS} PROPERTIES COMPILE_FLAGS "-w")

# Disable liblqr C code warnings.
SET_SOURCE_FILES_PROPERTIES(${liblensfun_SRCS} PROPERTIES COMPILE_FLAGS "-w")

SET(digikamcore_LIB_SRCS
        # basic libs
        ${libdimg_SRCS}
        ${libdimgloaders_SRCS}
        ${libdimgfilters_SRCS}
        ${libdmetadata_SRCS}
        ${libdatabasecore_SRCS}
        ${liblprof_SRCS}
        ${libjpegutils_SRCS}
        ${libthreadimageio_SRCS}
        ${libthememanager_SRCS}
        ${libpgf_SRCS}
        ${pgfutils_SRCS}
        ${libdigikamhelpers_SRCS}
        ${libtthread_SRCS}
        ${libversionmanager_SRCS}
        ${libkgeomaphelper_SRCS}
        ${libkmemoryinfo_SRCS}

        # widgets and dialogs
        ${libcommonwidgets_SRCS}
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
        ${libdimgcanvas_SRCS}
        ${libdimgeditorprintiface_SRCS}
        ${libdimgeditor_SRCS}
        ${librawimport_SRCS}
)

KDE4_ADD_UI_FILES(digikamcore_LIB_SRCS
                  ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/printiface/printoptionspage.ui
                 )

KDE4_ADD_KCFG_FILES(digikamcore_LIB_SRCS
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/printiface/digikamconfig.kcfgc
                   )

KDE4_ADD_LIBRARY(digikamcore SHARED ${digikamcore_LIB_SRCS})

ADD_DEPENDENCIES(digikamcore digikam-gitversion)

TARGET_LINK_LIBRARIES(digikamcore
        ${QT_QT3SUPPORT_LIBRARY}
        ${QT_QTSQL_LIBRARY}

        ${KDE4_KDECORE_LIBS}
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4_KHTML_LIBS}
        ${KDE4_KNOTIFYCONFIG_LIBRARY}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KUTILS_LIBS}
        ${KDE4_SOLID_LIBS}

        ${JPEG_LIBRARIES}
        ${TIFF_LIBRARIES}
        ${PNG_LIBRARIES}
        ${LCMS_LIBRARIES}
        ${JASPER_LIBRARIES}
        ${OPENMP_LDFLAGS}

        # Required by CImg library which is using pthread internally.
        ${CMAKE_THREAD_LIBS_INIT}

        ${KDCRAW_LIBRARIES}
        ${KEXIV2_LIBRARIES}
        ${KIPI_LIBRARIES}
        ${KFACE_LIBRARIES}
        ${KGEOMAP_LIBRARIES}
)

IF(USE_SCRIPT_IFACE)
    TARGET_LINK_LIBRARIES(digikamcore ${QT_QTSCRIPT_LIBRARY})
ENDIF(USE_SCRIPT_IFACE)

IF(PGF_FOUND)
    TARGET_LINK_LIBRARIES(digikamcore ${PGF_LIBRARIES})
ENDIF(PGF_FOUND)

IF(CLAPACK_FOUND)
    TARGET_LINK_LIBRARIES(digikamcore ${CLAPACK_LIBRARY})
ENDIF(CLAPACK_FOUND)

IF(Q_WS_X11)
    TARGET_LINK_LIBRARIES(digikamcore ${X11_LIBRARIES})
ENDIF(Q_WS_X11)

# For Contents Aware Resizer and Lens auto-correction tools.
IF(GLIB2_FOUND)
    TARGET_LINK_LIBRARIES(digikamcore ${GLIB2_LIBRARIES} ${liblqr_LIBS} ${liblensfun_LIBS})
ENDIF(GLIB2_FOUND)

SET_TARGET_PROPERTIES(digikamcore PROPERTIES VERSION 2.0.0 SOVERSION 2)
IF(WIN32)
    SET_TARGET_PROPERTIES(digikamcore PROPERTIES COMPILE_FLAGS -DJPEG_STATIC)
    SET_TARGET_PROPERTIES(digikamcore PROPERTIES OUTPUT_NAME digikamlib)
ENDIF(WIN32)
INSTALL(TARGETS digikamcore ${INSTALL_TARGETS_DEFAULT_ARGS})


# Shared libdigikamdatabase ########################################################

# Disable libsqlite2 C code warnings plus specific flags.
IF(WIN32)
    SET_SOURCE_FILES_PROPERTIES(${libsqlite2_SRCS} PROPERTIES COMPILE_FLAGS "-w -D_OFF_T_ -D_OFF_T_DEFINED")
ELSE(WIN32)
    SET_SOURCE_FILES_PROPERTIES(${libsqlite2_SRCS} PROPERTIES COMPILE_FLAGS "-w")
ENDIF(WIN32)

## Disable libpgf warnings.
#SET_SOURCE_FILES_PROPERTIES(${libpgf_SRCS} PROPERTIES COMPILE_FLAGS "-w")

SET(digikamdatabase_LIB_SRCS
        ${libdatabase_SRCS}
        ${libhaar_SRCS}
        ${libdatabasemodels_SRCS}
        ${libsqlite2_SRCS}
        ${libpgf_SRCS}
        ${pgfutils_SRCS}
)

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
                      ${KFACE_LIBRARIES}
                      ${KGEOMAP_LIBRARIES}

                      ${OPENMP_LDFLAGS}
                     )

IF(USE_SCRIPT_IFACE)
    TARGET_LINK_LIBRARIES(digikamdatabase ${QT_QTSCRIPT_LIBRARY})
ENDIF(USE_SCRIPT_IFACE)

IF(PGF_FOUND)
    TARGET_LINK_LIBRARIES(digikamdatabase ${PGF_LIBRARIES})
ENDIF(PGF_FOUND)

SET_TARGET_PROPERTIES(digikamdatabase PROPERTIES VERSION 2.0.0 SOVERSION 2)

INSTALL(TARGETS digikamdatabase ${INSTALL_TARGETS_DEFAULT_ARGS} )

# digiKam nepomuk services #########################################################

IF (NEPOMUK_FOUND)

    SET(digikamnepomukservices_SRCS ${libnepomukservices_SRCS} )

    INCLUDE(SopranoAddOntology)
    SOPRANO_ADD_ONTOLOGY(digikamnepomukservices_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
    SOPRANO_ADD_ONTOLOGY(digikamnepomukservices_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")

    KDE4_ADD_PLUGIN(digikamnepomukservice ${digikamnepomukservices_SRCS})

    TARGET_LINK_LIBRARIES(digikamnepomukservice
                          ${QT_QTSQL_LIBRARY}

                          ${KDE4_KIO_LIBS}
                          ${KDE4_SOLID_LIBS}
                          ${KDE4_KDEUI_LIBS}

                          ${KIPI_LIBRARIES}
                          ${KEXIV2_LIBRARIES}
                          ${KDCRAW_LIBRARIES}
                          ${SOPRANO_CLIENT_LIBRARIES}
                          ${SOPRANO_LIBRARIES}
                          ${NEPOMUK_LIBRARIES}

                          digikamcore
                          digikamdatabase
                         )

    IF(USE_SCRIPT_IFACE)
        TARGET_LINK_LIBRARIES(digikamnepomukservice ${QT_QTSCRIPT_LIBRARY})
    ENDIF(USE_SCRIPT_IFACE)

    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/nepomuk/digikamnepomukservice.desktop
            DESTINATION ${SERVICES_INSTALL_DIR})

    INSTALL(TARGETS digikamnepomukservice
            DESTINATION ${PLUGIN_INSTALL_DIR})

ENDIF (NEPOMUK_FOUND)

# digiKam binary progam #########################################################

SET(digikam_SRCS ${libdimgimagewindow_SRCS}
                 ${libdeletedialog_SRCS}
                 ${libimagepropertiesdigikam_SRCS}
                 ${libimageversionssidebar_SRCS}
                 ${libsetup_SRCS}
                 ${libcameragui_SRCS}
                 ${liblighttable_SRCS}
                 ${libbatch_SRCS}
                 ${libkipiiface_SRCS}
                 ${libsearchwindow_SRCS}
                 ${libfuzzysearch_SRCS}
                 ${libgpssearch_SRCS}
                 ${libkgeomaphelper_SRCS}
                 ${libqueuemanager_SRCS}
                 ${libalbummodels_SRCS}
                 ${libdigikamgui_SRCS}
                 ${libfirstrun_SRCS}
                 ${libtemplate_SRCS}
                 ${libdatabaseui_SRCS}
                 ${libfacedetection_SRCS}
                 main/main.cpp
)

IF(USE_SCRIPT_IFACE)
    SET(digikam_SRCS ${digikam_SRCS} ${libscriptiface_SRCS})
ENDIF(USE_SCRIPT_IFACE)

QT4_ADD_DBUS_ADAPTOR(digikam_SRCS main/org.kde.digikam.xml main/digikamapp.h Digikam::DigikamApp)

KDE4_ADD_APP_ICON(digikam_SRCS "../data/icons/apps/hi*-app-digikam.png")

KDE4_ADD_EXECUTABLE(digikam ${digikam_SRCS})

TARGET_LINK_LIBRARIES(digikam
        ${QT_QTSQL_LIBRARY}
        ${QT_QT3SUPPORT_LIBRARY}

        ${KDE4_KPARTS_LIBS}
        ${KDE4_PHONON_LIBS}
        ${KDE4_KHTML_LIBS}
        ${KDE4_KUTILS_LIBS}
        ${KDE4_KNOTIFYCONFIG_LIBRARY}
        ${KDE4_SOLID_LIBS}

        ${KFACE_LIBRARIES}
        ${KGEOMAP_LIBRARIES}
        ${KIPI_LIBRARIES}
        ${KEXIV2_LIBRARIES}
        ${KDCRAW_LIBRARIES}

        ${STRIGI_LIBRARIES}
        ${LCMS_LIBRARIES}

        digikamcore
        digikamdatabase
        advancedrename
        queuemanager
        cameragui

        # See B.K.O #258931: libgphoto2 library must be the last arg for linker.
        ${GPHOTO2_LIBRARIES}
)

IF (NOT WIN32 AND GPHOTO2_FOUND)
    # See B.K.O #268267 : digiKam need to be linked to libusb to prevent crash at gphoto2 init if opencv is linked with libdc1394
    TARGET_LINK_LIBRARIES(digikam ${LIBUSB_LIBRARIES})
ENDIF (NOT WIN32 AND GPHOTO2_FOUND)


IF(USE_SCRIPT_IFACE)
    TARGET_LINK_LIBRARIES(digikam ${QT_QTSCRIPT_LIBRARY})
ENDIF(USE_SCRIPT_IFACE)

IF(KDEPIMLIBS_FOUND)
  TARGET_LINK_LIBRARIES(digikam ${KDE4_KABC_LIBS})
ENDIF(KDEPIMLIBS_FOUND)

IF(CLAPACK_FOUND)
   TARGET_LINK_LIBRARIES(digikam ${CLAPACK_LIBRARY})
ENDIF(CLAPACK_FOUND)

INSTALL(TARGETS digikam ${INSTALL_TARGETS_DEFAULT_ARGS})

# Others Files to install #######################################################

INSTALL(PROGRAMS main/digikam.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL(FILES main/digikamui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES main/digikam.notifyrc DESTINATION ${DATA_INSTALL_DIR}/digikam)
