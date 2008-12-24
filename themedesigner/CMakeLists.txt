
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
                     ../digikam
                     ../

                     # config-digikam.h
                     ${CMAKE_CURRENT_BINARY_DIR}/../

                     # external paths
                     ${LCMS_INCLUDE_DIR}
                   )

SET(libdigikamtheme_SRCS 
    ../digikam/album.cpp 
    ../digikam/albumiconview.cpp 
    ../digikam/albumicongroupitem.cpp 
    ../digikam/albumiconitem.cpp 
    ../digikam/albumitemhandler.cpp 
    ../digikam/albumiconviewtooltip.cpp 
    ../digikam/albumlister.cpp 
    ../digikam/albummanager.cpp 
    ../digikam/albumsettings.cpp 
    ../digikam/albumselectdialog.cpp 
    ../digikam/albumthumbnailloader.cpp 
    ../digikam/cameralist.cpp 
    ../digikam/cameratype.cpp 
    ../digikam/dio.cpp 
    ../digikam/ddragobjects.cpp 
    ../digikam/folderitem.cpp 
    ../digikam/folderview.cpp 
    ../digikam/iconview.cpp 
    ../digikam/icongroupitem.cpp 
    ../digikam/iconitem.cpp 
    ../digikam/imageattributeswatch.cpp 
    ../digikam/imagepreviewbar.cpp 
    ../digikam/kdateedit.cpp 
    ../digikam/kdatetimeedit.cpp 
    ../digikam/kdatepickerpopup.cpp 
    ../digikam/metadatahub.cpp 
    ../digikam/ratingpopupmenu.cpp 
    ../digikam/ratingwidget.cpp 
    ../digikam/scancontroller.cpp
    ../digikam/syncjob.cpp 
    ../digikam/tageditdlg.cpp 
    ../digikam/tagspopupmenu.cpp 
    ../digikam/treefolderitem.cpp 
   )

SET(digikamthemedesigner_SRCS 
    ${libdigikamtheme_SRCS}
    ${libdeletedialog_SRCS}
    ${liblighttable_SRCS}
    ${libdimgimagewindow_SRCS}
    ${libimagepropertiesdigikam_SRCS}
    ${libcameragui_SRCS}
    ${libsetup_SRCS}
    ${libbatch_SRCS}

    main.cpp 
    mainwindow.cpp 
    themedicongroupitem.cpp 
    themediconitem.cpp 
    themediconview.cpp 
   )

KDE4_ADD_EXECUTABLE(digikamthemedesigner ${digikamthemedesigner_SRCS})

TARGET_LINK_LIBRARIES(digikamthemedesigner 
                      digikamcore 
                      digikamdatabase 
                      ${KDE4_KUTILS_LIBS}
                      ${KIPI_LIBRARIES}
                      ${GPHOTO2_LIBRARIES}
                     )

IF(KDEPIMLIBS_FOUND)
    TARGET_LINK_LIBRARIES(digikamthemedesigner ${KDE4_KABC_LIBS})
ENDIF(KDEPIMLIBS_FOUND)

INSTALL(TARGETS digikamthemedesigner ${INSTALL_TARGETS_DEFAULT_ARGS})

