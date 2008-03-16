
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
                     ${CMAKE_BINARY_DIR}/digikam/digikam

                     # external paths
                     ${LCMS_INCLUDE_DIR}
                   )

SET(libdigikamtheme_SRCS 
    ../digikam/album.cpp 
    ../digikam/albumiconview.cpp 
    ../digikam/albumicongroupitem.cpp 
    ../digikam/albumiconitem.cpp 
    ../digikam/albumitemhandler.cpp 
    ../digikam/albumfiletip.cpp 
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
    ../digikam/kdateedit.cpp 
    ../digikam/kdatetimeedit.cpp 
    ../digikam/kdatepickerpopup.cpp 
    ../digikam/metadatahub.cpp 
    ../digikam/ratingpopupmenu.cpp 
    ../digikam/ratingwidget.cpp 
    ../digikam/scancontroller.cpp
    ../digikam/scanlib.cpp 
    ../digikam/syncjob.cpp 
    ../digikam/tagcreatedlg.cpp 
    ../digikam/tagspopupmenu.cpp 
    ../digikam/treefolderitem.cpp 
   )

SET(digikamthemedesigner_SRCS 
    ${libdigikamtheme_SRCS}
    ${libthemeengine_SRCS}
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

KDE4_ADD_UI_FILES(digikamthemedesigner_SRCS ../libs/dialogs/deletedialogbase.ui)

KDE4_ADD_EXECUTABLE(digikamthemedesigner ${digikamthemedesigner_SRCS})

TARGET_LINK_LIBRARIES(digikamthemedesigner 
                      digikam 
                      digikamdatabase 
                      ${KDE4_KUTILS_LIBS}
                      ${KDE4_KABC_LIBS}
                      ${GPHOTO2_LIBRARIES}
                     )

INSTALL(TARGETS digikamthemedesigner DESTINATION ${BIN_INSTALL_DIR})

