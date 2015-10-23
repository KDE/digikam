#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_version.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_version.h)

# digikam core
set(DIGIKAMCORE_OBJECTS
            $<TARGET_OBJECTS:digikamdatabasecore_src>
            $<TARGET_OBJECTS:dimg_src>
            $<TARGET_OBJECTS:dmetadata_src>
            $<TARGET_OBJECTS:jpegutils_src>
            $<TARGET_OBJECTS:progressmanager_src>
            $<TARGET_OBJECTS:threadimageio_src>
            $<TARGET_OBJECTS:pgfutils_src>
            $<TARGET_OBJECTS:dthread_src>
            $<TARGET_OBJECTS:versionmanager_src>
            $<TARGET_OBJECTS:3rdparty_src>
            $<TARGET_OBJECTS:dngwriter_src>
            $<TARGET_OBJECTS:kiowrapper_src>
            $<TARGET_OBJECTS:rawengine_srcs>

            # widgets
            $<TARGET_OBJECTS:digikamwidgetscore_src>
            $<TARGET_OBJECTS:digikamdialogscore_src>
            $<TARGET_OBJECTS:imageproperties_src>
            $<TARGET_OBJECTS:digikamgenericmodels_src>

            # utilities
            $<TARGET_OBJECTS:slideshow_src>
            $<TARGET_OBJECTS:imageeditor_src>
            $<TARGET_OBJECTS:editorplugin_src>
            $<TARGET_OBJECTS:metadataedit_src>
            $<TARGET_OBJECTS:akonadiiface_src>  # NOTE: even if optional this code need to be compiled here.

            utils/digikam_debug.cpp
)

if(MARBLE_FOUND)
    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:geoiface_src>
        $<TARGET_OBJECTS:geomapwrapper_src>
        $<TARGET_OBJECTS:geolocationedit_src>
    )
endif()

if(KF5Sane_FOUND)
    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:ksaneiface_src>
    )
endif()

include_directories(
    $<TARGET_PROPERTY:KF5::Kipi,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Completion,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::WindowSystem,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_MEDIAPLAYER)
    include_directories(
        $<TARGET_PROPERTY:Qt5::Multimedia,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Qt5::MultimediaWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

if(KF5KFace_FOUND)
    include_directories(
        $<TARGET_PROPERTY:${KF5KFace_LIBRARIES},INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

if(GPHOTO2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

add_library(digikamcore
            SHARED
            ${DIGIKAMCORE_OBJECTS}
)

target_link_libraries(digikamcore
                      PRIVATE

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::WebKitWidgets
                      Qt5::PrintSupport
                      Qt5::Concurrent

                      KF5::Solid
                      KF5::KIOCore
                      KF5::NotifyConfig
                      KF5::WindowSystem
                      KF5::CoreAddons
                      KF5::Service
                      KF5::KIOWidgets
                      KF5::IconThemes
                      KF5::I18n

                      # Required by CImg which use pthread internally.

                      ${CMAKE_THREAD_LIBS_INIT}

                      ${LCMS2_LIBRARIES} # filters

                      ${TIFF_LIBRARIES}
                      ${PNG_LIBRARIES}
                      ${EXIV2_LIBRARIES}
                      ${PGF_LIBRARIES}

                      libdng
                      libraw
)

if(KF5Kipi_FOUND)
    target_link_libraries(digikamcore PRIVATE KF5::Kipi)
endif()

if(MARBLE_FOUND)
    target_link_libraries(digikamcore PRIVATE ${MARBLE_LIBRARIES}
                                              KF5::ItemModels)

    if(KF5Bookmarks_FOUND)
        target_link_libraries(digikamcore PRIVATE KF5::Bookmarks)
    endif()
endif()

if(X11_FOUND)
    target_link_libraries(digikamcore PRIVATE Qt5::X11Extras ${X11_LIBRARIES})
endif()

if(JASPER_FOUND)
    target_link_libraries(digikamcore PRIVATE ${JASPER_LIBRARIES})
endif()

# LibLqr-1 library rules for content-aware filter
if(LQR-1_FOUND)
    target_link_libraries(digikamcore PRIVATE ${LQR-1_LIBRARIES})
endif()

if(LENSFUN_FOUND)
    target_include_directories(digikamcore PUBLIC ${LENSFUN_INCLUDE_DIRS})
    target_link_libraries(digikamcore PRIVATE ${LENSFUN_LIBRARIES})
endif()

# for nrfilter
if(OpenCV_FOUND)
    target_link_libraries(digikamcore PRIVATE ${OpenCV_LIBRARIES})
endif()

if(${KFILEMETADATA_FOUND})
    target_link_libraries(digikamcore PRIVATE baloowrap)
endif()

if(ENABLE_MEDIAPLAYER)
    target_link_libraries(digikamcore PRIVATE Qt5::Multimedia Qt5::MultimediaWidgets)
endif()

if(APPLE)
    target_link_libraries(digikamcore PUBLIC /System/Library/Frameworks/AppKit.framework)
endif()

if(${KF5KFace_FOUND})
    target_link_libraries(digikamcore PRIVATE ${KF5KFace_LIBRARIES})
endif()

if(${KF5Sane_FOUND})
    target_link_libraries(digikamcore
                          PUBLIC
                          ${LIBKSANE_LIBRARIES}
                         )
endif()

if(KF5AkonadiContact_FOUND)
    target_link_libraries(digikamcore PRIVATE KF5::AkonadiContact)
endif()

set_target_properties(digikamcore PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

install(TARGETS digikamcore ${INSTALL_TARGETS_DEFAULT_ARGS})

########################## digiKam GUI sources #############################

qt5_add_dbus_adaptor(digikamadaptor_SRCS main/org.kde.digikam.xml main/digikamapp.h Digikam::DigikamApp)

set(libdigikamgui_SRCS
    main/digikamapp.cpp

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

    filters/filtersidebarwidget.cpp
    filters/tagfilterview.cpp

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
    items/overlays/imagefsoverlay.cpp
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
    utils/digikam_debug.cpp

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

    views/imagepreviewviewitem.cpp
    views/imagepreviewview.cpp
    views/welcomepageview.cpp
    views/leftsidebarwidgets.cpp
    views/digikamview.cpp
    views/trashview.cpp
    views/stackedview.cpp

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

if(${MARBLE_FOUND})
    set(libdigikamgui_SRCS
        ${libdigikamgui_SRCS}
        views/mapwidgetview.cpp
       )
endif()


add_library(digikamgui_src
            OBJECT
            ${libdigikamgui_SRCS}
)


######################### digiKam GUI objects ############################


# this is only required by Win & OSX
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../data/icons/apps/*-apps-digikam.png")
ecm_add_app_icon(digikam_SRCS ICONS ${ICONS_SRCS})

set(DIGIKAM_OBJECTS
            $<TARGET_OBJECTS:digikamdatabasemain_src>
            $<TARGET_OBJECTS:digikamgui_src>
            $<TARGET_OBJECTS:digikamdeletedialog_src>
            $<TARGET_OBJECTS:digikamtemplate_src>
            $<TARGET_OBJECTS:imagepropertiesdigikam_src>
            $<TARGET_OBJECTS:setup_src>
            $<TARGET_OBJECTS:lighttable_src>
            $<TARGET_OBJECTS:maintenance_src>
            $<TARGET_OBJECTS:searchwindow_src>
            $<TARGET_OBJECTS:digikammodels_src>
            $<TARGET_OBJECTS:digikamalbum_src>
            $<TARGET_OBJECTS:firstrun_src>
            $<TARGET_OBJECTS:fuzzysearch_src>
            $<TARGET_OBJECTS:imageeditorgui_src>
            $<TARGET_OBJECTS:fileactionmanager_src>
            $<TARGET_OBJECTS:digikamtags_src>
            $<TARGET_OBJECTS:digikamsettings_src>
            $<TARGET_OBJECTS:notificationmanager_src>
            $<TARGET_OBJECTS:filters_src>
            $<TARGET_OBJECTS:importuibackend_src>
            $<TARGET_OBJECTS:imagehistorywidgets_src>
            $<TARGET_OBJECTS:iojobs_src>
            $<TARGET_OBJECTS:dtrash_src>
)

if(${KF5Kipi_FOUND})
    set(DIGIKAM_OBJECTS
        ${DIGIKAM_OBJECTS}
        $<TARGET_OBJECTS:kipiiface_src>
    )
endif()

if(${KF5KFace_FOUND})
    set(DIGIKAM_OBJECTS
        ${DIGIKAM_OBJECTS}
            $<TARGET_OBJECTS:facemanagement_src>
    )
endif()

if(${MARBLE_FOUND})
    set(DIGIKAM_OBJECTS
        ${DIGIKAM_OBJECTS}
            $<TARGET_OBJECTS:geomapwrapper_src>
            $<TARGET_OBJECTS:gpssearch_src>
    )
endif()

#################### Digikam GUI shared Lib ################################

add_library(digikamgui
            SHARED
            ${DIGIKAM_OBJECTS}
)

target_link_libraries(digikamgui

                      digikamdatabase
                      digikamcore

                      queuemanager
                      advancedrename
                      importui

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::WebKitWidgets
                      Qt5::PrintSupport
                      Qt5::DBus

                      KF5::Solid
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::Notifications
                      KF5::NotifyConfig
                      KF5::Service
                      KF5::WindowSystem
                      KF5::IconThemes
                      KF5::I18n

                      ${OpenCV_LIBRARIES}
)

if(${KF5KFace_FOUND})
    target_link_libraries(digikamgui ${KF5KFace_LIBRARIES})
endif()

if(${KF5Kipi_FOUND})
    target_link_libraries(digikamgui KF5::Kipi)
endif()

if(${MARBLE_FOUND})
     target_link_libraries(digikamgui ${MARBLE_LIBRARIES})
endif()

if(APPLE)
    target_link_libraries(digikamgui /System/Library/Frameworks/AppKit.framework)
endif()

if(ENABLE_MEDIAPLAYER)
    target_link_libraries(digikamgui Qt5::Multimedia Qt5::MultimediaWidgets)
endif()

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(digikamgui ${MATH_LIBRARY})
endif()

if(WIN32)
    set_target_properties(digikamgui PROPERTIES COMPILE_FLAGS -DJPEG_STATIC)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(digikamgui ${KVM_LIBRARY})
endif()

if(GPHOTO2_FOUND)
    # See bug #258931: libgphoto2 library must be the last arg for linker.
    target_link_libraries(digikamgui ${GPHOTO2_LIBRARIES})

    if(NOT WIN32)
        # See bug #268267 : digiKam need to be linked to libusb to prevent crash at gphoto2 init if opencv is linked with libdc1394
        target_link_libraries(digikamgui ${LibUSB1_LIBRARIES})
    endif()
endif()

set_target_properties(digikamgui PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

install(TARGETS digikamgui ${INSTALL_TARGETS_DEFAULT_ARGS})

#################### digiKam Executable  ####################################

set(digikam_SRCS
    main/main.cpp
)

add_executable(digikam ${digikam_SRCS})

add_dependencies(digikam digikam-gitversion)

# TO fill plist XML file for OSX ############

set(MACOSX_APP_NAME_STRING             "digikam")
set(MACOSX_APP_DESCRIPTION             "Advanced digital photo management application")
set(MACOSX_BUNDLE_LONG_VERSION_STRING  ${DIGIKAM_VERSION_STRING})
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${DIGIKAM_VERSION_SHORT})
set(MACOSX_BUNDLE_BUNDLE_VERSION       ${DIGIKAM_VERSION_STRING})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/Info.plist.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
set_target_properties(digikam PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

target_link_libraries(digikam

                      digikamgui

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::DBus

                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::WindowSystem
                      KF5::IconThemes
                      KF5::I18n
)

install(TARGETS digikam ${INSTALL_TARGETS_DEFAULT_ARGS})

# Others Files to install #######################################################

install(PROGRAMS main/digikam.desktop     DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES    main/digikam.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)
install(FILES    main/digikamui.rc        DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES    main/digikam.notifyrc    DESTINATION ${KNOTIFYRC_INSTALL_DIR})
