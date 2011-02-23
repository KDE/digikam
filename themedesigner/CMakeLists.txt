SET(digikamthemedesigner_SRCS
        ${libalbummodels_SRCS}
        ${libbatch_SRCS}
        ${libdatabaseui_SRCS}
        ${libdeletedialog_SRCS}
        ${libdigikamgui_SRCS}
        ${libdimgimagewindow_SRCS}
        ${libfacedetection_SRCS}
        ${libfuzzysearch_SRCS}
        ${libgpssearch_SRCS}
        ${libimagepropertiesdigikam_SRCS}
        ${libimageversionssidebar_SRCS}
        ${libkipiiface_SRCS}
        ${liblighttable_SRCS}
        ${libscriptiface_SRCS}
        ${libsearchwindow_SRCS}
        ${libsetup_SRCS}
        ${libtemplate_SRCS}

        main.cpp
        mainwindow.cpp
        themedicongroupitem.cpp
        themediconitem.cpp
        themediconview.cpp
)

QT4_ADD_DBUS_ADAPTOR(digikamthemedesigner_SRCS
                     ../digikam/org.kde.digikam.xml digikamapp.h Digikam::DigikamApp)

KDE4_ADD_EXECUTABLE(digikamthemedesigner ${digikamthemedesigner_SRCS})

TARGET_LINK_LIBRARIES(digikamthemedesigner
        digikamcore
        digikamdatabase
        advancedrename
        queuemanager
        cameragui
        ${KDE4_KPARTS_LIBS}
        ${KDE4_PHONON_LIBS}
        ${KDE4_KHTML_LIBS}
        ${KDE4_KUTILS_LIBS}
        ${KDE4_SOLID_LIBS}
        ${KIPI_LIBRARIES}
        ${KEXIV2_LIBRARIES}
        ${KDCRAW_LIBRARIES}
        ${QT_QT3SUPPORT_LIBRARY}
        ${STRIGI_LIBRARIES}
        ${LCMS_LIBRARIES}
        ${GPHOTO2_LIBRARIES}
)

IF(KDEPIMLIBS_FOUND)
    TARGET_LINK_LIBRARIES(digikamthemedesigner ${KDE4_KABC_LIBS})
ENDIF(KDEPIMLIBS_FOUND)

INSTALL(TARGETS digikamthemedesigner ${INSTALL_TARGETS_DEFAULT_ARGS})
