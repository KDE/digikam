
INCLUDE_DIRECTORIES(
                     # basic libs
                     ../libs/dimg
                     ../libs/dimg/loaders
                     ../libs/dimg/filters
                     ../libs/dmetadata 
                     ../libs/histogram
                     ../libs/curves
                     ../libs/levels
                     ../libs/lprof/
                     ../libs/jpegutils 
                     ../libs/greycstoration
                     ../libs/threadimageio
                     ../libs/whitebalance

                     # widgets and dialogs
                     ../libs/widgets/common
                     ../libs/widgets/imageplugins 
                     ../libs/widgets/metadata 
                     ../libs/widgets/iccprofiles 
                     ../libs/imageproperties
                     ../libs/dialogs

                     # database libs
                     ../libs/database
                     ../libs/database/sqlite2
                     ../libs/database/haar

                     # shared utilities
                     ../utilities/slideshow
                     ../utilities/imageeditor/editor
                     ../utilities/imageeditor/canvas
                     ../utilities/imageeditor/tools

                     # specific libs and utilities
                     ../libs/themeengine
                     ../utilities/cameragui
                     ../utilities/setup
                     ../utilities/slideshow
                     ../utilities/batch
                     ../utilities/lighttable
                     ../utilities/kipiiface
                     ../utilities/searchwindow
                     ../utilities/fuzzysearch
                     ../utilities/gpssearch
                     ../utilities/timeline
                     ../utilities/queuemanager
                     ../utilities/queuemanager/basetools/convert
                     ../utilities/queuemanager/basetools/enhance
                     ../utilities/queuemanager/basetools/transform
                     ../utilities/queuemanager/basetools/metadata
                     ../utilities/queuemanager/basetools/decorate
                     ../digikam
                     ../

                     # config-digikam.h
                     ${CMAKE_CURRENT_BINARY_DIR}/../

                     # external paths
                     ${LCMS_INCLUDE_DIR}
                   )

SET(digikamthemedesigner_SRCS 
    ${libdimgimagewindow_SRCS}
    ${libdeletedialog_SRCS}
    ${libimagepropertiesdigikam_SRCS}
    ${libsetup_SRCS}
    ${libcameragui_SRCS}
    ${liblighttable_SRCS}
    ${libbatch_SRCS}
    ${libkipiiface_SRCS}
    ${libsearchwindow_SRCS}
    ${libfuzzysearch_SRCS}
    ${libgpssearch_SRCS}
    ${libtimeline_SRCS}
    ${libdigikamgui_SRCS}
    ${libqueuemanager_SRCS}

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

IF(MARBLEWIDGET_FOUND)
    TARGET_LINK_LIBRARIES(digikamthemedesigner ${LIBMARBLEWIDGET_LIBRARY})
ENDIF(MARBLEWIDGET_FOUND)

INSTALL(TARGETS digikamthemedesigner ${INSTALL_TARGETS_DEFAULT_ARGS})

