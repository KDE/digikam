
INCLUDE_DIRECTORIES(
                     # basic libs
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/histogram    
                     ${CMAKE_SOURCE_DIR}/digikam/libs/curves
                     ${CMAKE_SOURCE_DIR}/digikam/libs/levels
                     ${CMAKE_SOURCE_DIR}/digikam/libs/lprof/
                     ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/greycstoration
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio

                     # widgets and dialogs
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs

                     # database libs
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database/sqlite2

                     # shared utilities
                     ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/tools

                     # specific libs and utilities
                     ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/cameragui
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/setup
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/batch
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/lighttable
                     ${CMAKE_SOURCE_DIR}/digikam/digikam
                   )

SET(libdigikamtheme_SRCS 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/album.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albumiconview.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albumicongroupitem.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albumiconitem.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albumitemhandler.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albumfiletip.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albumlister.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albummanager.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albumsettings.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/albumthumbnailloader.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/cameradragobject.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/cameralist.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/cameratype.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/dio.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/dragobjects.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/folderitem.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/folderview.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/iconview.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/icongroupitem.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/iconitem.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/imageattributeswatch.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/kdateedit.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/kdatetimeedit.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/kdatepickerpopup.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/metadatahub.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/pixmapmanager.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/ratingpopupmenu.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/ratingwidget.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/scanlib.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/syncjob.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/tagcreatedlg.cpp 
    ${CMAKE_SOURCE_DIR}/digikam/digikam/tagspopupmenu.cpp 
   )

SET(digikamthemedesigner_SRCS 
    ${libdigikamtheme_SRCS}
    ${libthemeengine_SRCS}
    ${libthumbbar_SRCS}
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

KDE4_ADD_UI_FILES(digikamthemedesigner_SRCS ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs/deletedialogbase.ui)

KDE4_ADD_EXECUTABLE(digikamthemedesigner ${digikamthemedesigner_SRCS})

TARGET_LINK_LIBRARIES(digikamthemedesigner 
                      digikam 
                      digikamdatabase 
                      ${KDE4_KUTILS_LIBS}
                      ${KDE4_KABC_LIBS}
                      ${GPHOTO2_LIBRARIES}
                     )

INSTALL(TARGETS digikamthemedesigner DESTINATION ${BIN_INSTALL_DIR})

INSTALL(FILES digikamthemedesigner.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
