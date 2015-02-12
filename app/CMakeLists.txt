#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_version.h.cmake.in    ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_version.h)

# Shared libdigikamcore ################################################################

# TODO clean up this..
set(libdigikamhelpers_SRCS
    utils/uifilevalidator.cpp
)

set(digikamcore_LIB_SRCS
    ${libdigikamhelpers_SRCS}
)
# TODO this is now just a placeholder holding uifilevalidator.. to be changed later to expose a dynamic library for other apps to use?
add_library(digikamcore STATIC ${digikamcore_LIB_SRCS})

target_link_libraries(digikamcore
                      Qt5::Core
                      Qt5::Xml
)

set_target_properties(digikamcore PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

if(WIN32)
    set_target_properties(digikamcore PROPERTIES COMPILE_FLAGS -DJPEG_STATIC)
    set_target_properties(digikamcore PROPERTIES OUTPUT_NAME   digikamlib)
endif()

install(TARGETS digikamcore ${INSTALL_TARGETS_DEFAULT_ARGS})

# Digikam GUI begins here

include_directories(
    album/
    database/
    date/
    dragdrop/
    items/
    items/overlays/
    fileaction/
    views/
    views/tableview/
    utils/
    tags/
    tags/tagsmanager/
    tags/tagsmanager/models/
    main/
    ${CMAKE_CURRENT_BINARY_DIR}/app/utils # for qt5_add_dbus_adaptor adaptor, TODO
)

qt5_add_dbus_adaptor(digikamadaptor_SRCS main/org.kde.digikam.xml main/digikamapp.h Digikam::DigikamApp)

# TODO split this target
# TODO some parts could/should be their own libraries, like album access?
set(libdigikamgui_SRCS
    main/digikamapp.cpp

    album/album.cpp
    album/albumhistory.cpp
    album/albummanager.cpp
    album/albummodificationhelper.cpp
    album/albumpropsedit.cpp
    album/albumselectors.cpp
    album/albumselectcombobox.cpp
    album/albumselectdialog.cpp
    album/albumselectiontreeview.cpp
    album/albumselectwidget.cpp
    album/albumthumbnailloader.cpp
    album/albumtreeview.cpp
    album/albumwatch.cpp
    album/albumlabelstreeview.cpp

    database/databaseguierrorhandler.cpp
    database/dbstatdlg.cpp
    database/scancontroller.cpp
    database/dio.cpp
    database/syncjob.cpp
    database/imageattributeswatch.cpp

    date/ddateedit.cpp
    date/ddatetimeedit.cpp
    date/ddatepickerpopup.cpp
    date/datefolderview.cpp
    date/monthwidget.cpp
    date/timelinewidget.cpp

    dragdrop/importdragdrop.cpp
    dragdrop/albumdragdrop.cpp
    dragdrop/ddragobjects.cpp
    dragdrop/imagedragdrop.cpp
    dragdrop/tagdragdrop.cpp

    items/digikamimageview.cpp
    items/digikamimageview_p.cpp
    items/digikamimagedelegate.cpp
    items/imagecategorizedview.cpp
    items/imagecategorydrawer.cpp
    items/imagedelegate.cpp
    items/imagethumbnailbar.cpp
    items/imagethumbnaildelegate.cpp
    items/imageviewutilities.cpp

    items/overlays/groupindicatoroverlay.cpp
    items/overlays/imageratingoverlay.cpp
    items/overlays/imagerotationoverlay.cpp
    items/overlays/imagecoordinatesoverlay.cpp
    items/overlays/imageselectionoverlay.cpp

    utils/contextmenuhelper.cpp
    utils/tooltipfiller.cpp
    utils/searchmodificationhelper.cpp
    utils/editablesearchtreeview.cpp
    utils/sidebarwidget.cpp
    utils/slideshowbuilder.cpp
    utils/digikammodelcollection.cpp
    utils/digikam_debug.cpp # TODO should this be here or elsewhere?

    views/tableview/tableview.cpp
    views/tableview/tableview_treeview.cpp
    views/tableview/tableview_treeview_delegate.cpp
    views/tableview/tableview_column_configuration_dialog.cpp
    views/tableview/tableview_model.cpp
    views/tableview/tableview_columns.cpp
    views/tableview/tableview_column_audiovideo.cpp
    views/tableview/tableview_column_file.cpp
    views/tableview/tableview_column_geo.cpp
    views/tableview/tableview_column_digikam.cpp
    views/tableview/tableview_column_item.cpp
    views/tableview/tableview_column_photo.cpp
    views/tableview/tableview_column_thumbnail.cpp
    views/tableview/tableview_columnfactory.cpp
    views/tableview/tableview_selection_model_syncer.cpp
    
    tags/tagsmanager/tagsmanager.cpp
    tags/tagsmanager/tagpropwidget.cpp
    tags/tagsmanager/taglist.cpp
    tags/tagsmanager/tagmngrtreeview.cpp
    tags/tagsmanager/models/tagmngrlistview.cpp
    tags/tagsmanager/models/tagmngrlistmodel.cpp
    tags/tagsmanager/models/tagmngrlistitem.cpp

    views/imagepreviewviewitem.cpp
    views/imagepreviewview.cpp
    views/welcomepageview.cpp
    views/leftsidebarwidgets.cpp
    views/digikamview.cpp
    views/stackedview.cpp

    tags/addtagscombobox.cpp
    tags/addtagscompletionbox.cpp
    tags/addtagslineedit.cpp
    tags/tagcheckview.cpp
    tags/tagfolderview.cpp
    tags/taggingaction.cpp
    tags/tagslineeditoverlay.cpp
    tags/tagspopupmenu.cpp
    tags/tageditdlg.cpp
    tags/tagmodificationhelper.cpp
    tags/tagsactionmngr.cpp
    
    ${digikamadaptor_SRCS}
)

if(ENABLE_MEDIAPLAYER)
    set(libdigikamgui_SRCS
        ${libdigikamgui_SRCS}
        views/mediaplayerview.cpp
       )
endif()

if(${KF5KFace_FOUND})
    set(libdigikamgui_SRCS
        ${libdigikamgui_SRCS}
        items/digikamimagefacedelegate.cpp
        items/overlays/assignnameoverlay.cpp
        items/overlays/facerejectionoverlay.cpp
       )
endif()

if(${KF5KGeoMap_FOUND})
    set(libdigikamgui_SRCS
        ${libdigikamgui_SRCS}
        views/mapwidgetview.cpp
       )
endif()

# TODO this should be turned from static to dynamic
add_library(digikamgui STATIC ${libdigikamgui_SRCS})

# TODO temporary ugly hack, the common utils used by other components (esp. utilities and libs) should be moved to libs..
target_include_directories(digikamgui PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/album> # move out
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/main>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/views>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tags>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tags/tagsmanager>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/items>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/items/overlays> # common too, move out
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/database>
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/date> # belongs to libs/widgets?
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dragdrop>
)

target_link_libraries(digikamgui
                      digikamcore
                      digikamwidgets
                      digikamdatabase
                      digikammodels
                      dimg
                      dthread
                      threadimageio
                      setup
                      imagepropertiesdigikam
                      advancedrename
                      queuemanager
                      importui
                      imageeditor
                      searchwindow
                      firstrun
                      lighttable
                      fuzzysearch
                      maintenance
                      3rdparty

                      fileactionmanager
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::WebKitWidgets
)

# TODO check if these linkings are required?
if(Baloo_FOUND)
    target_link_libraries(digikamgui baloowrap)
endif()

if(ENABLE_MEDIAPLAYER)
    target_link_libraries(digikamgui Qt5::Multimedia Qt5::MultimediaWidgets)
endif()

# For Contents Aware Resizer and Lens auto-correction tools.
if(GLIB2_FOUND)
    target_link_libraries(digikamgui ${GLIB2_LIBRARIES})
endif()

if(APPLE)
    target_link_libraries(digikamgui /System/Library/Frameworks/AppKit.framework)
endif()

if(${KF5Kipi_FOUND})
    target_link_libraries(digikamgui KF5::Kipi)
endif()


# digiKam binary progam #########################################################

set(digikam_SRCS
    main/main.cpp
)

# this is only required by Win & OSX
kde4_add_app_icon(digikam_SRCS "../data/icons/apps/hi*-app-digikam.png")

add_executable(digikam ${digikam_SRCS})

add_dependencies(digikam digikam-gitversion)

if(KF5KGeoMap_FOUND) # fixme hackish
    target_link_libraries(digikam gpssearch)
endif()

# TO fill plist XML file for OSX #####################################################

set(MACOSX_APP_NAME_STRING             "digikam")
set(MACOSX_APP_DESCRIPTION             "Advanced digital photo management application")
set(MACOSX_BUNDLE_LONG_VERSION_STRING  ${DIGIKAM_VERSION_STRING})
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${DIGIKAM_VERSION_SHORT})
set(MACOSX_BUNDLE_BUNDLE_VERSION       ${DIGIKAM_VERSION_STRING})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/Info.plist.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
set_target_properties(digikam PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)


target_link_libraries(digikam
                      digikamcore
                      digikamgui


                      Qt5::Sql
                      Qt5::Test
                      Qt5::WebKitWidgets
                      
                      ${OpenCV_LIBRARIES}
)
if(ENABLE_MEDIAPLAYER)
  target_link_libraries(digikam Qt5::Multimedia Qt5::MultimediaWidgets)
endif()

if(KDEPIMLIBS_FOUND)
  target_link_libraries(digikam ${KDE4_KABC_LIBS} KF5::AkonadiCore KF5::AkonadiContact)
endif()

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(digikam ${MATH_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(digikam ${KVM_LIBRARY})
endif()

if(GPHOTO2_FOUND)
    # See bug #258931: libgphoto2 library must be the last arg for linker.
    target_link_libraries(digikam ${GPHOTO2_LIBRARIES})
    
    if(NOT WIN32)
        # See bug #268267 : digiKam need to be linked to libusb to prevent crash at gphoto2 init if opencv is linked with libdc1394
        target_link_libraries(digikam ${LIBUSB_LIBRARIES})
    endif()
endif()

install(TARGETS digikam ${INSTALL_TARGETS_DEFAULT_ARGS})

# Others Files to install #######################################################

install(PROGRAMS main/digikam.desktop     DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES    main/digikam.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)
install(FILES    main/digikamui.rc        DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES    main/digikam.notifyrc    DESTINATION ${KNOTIFYRC_INSTALL_DIR})
