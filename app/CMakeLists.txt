#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_version.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_version.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_dbconfig.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_dbconfig.h)

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
            $<TARGET_OBJECTS:kmemoryinfo_src>
            $<TARGET_OBJECTS:dngwriter_src>
            $<TARGET_OBJECTS:rawengine_srcs>
            $<TARGET_OBJECTS:digikamfacesengine_src>

            # widgets
            $<TARGET_OBJECTS:digikamwidgetscore_src>
            $<TARGET_OBJECTS:digikamdialogscore_src>
            $<TARGET_OBJECTS:imageproperties_src>
            $<TARGET_OBJECTS:digikamgenericmodels_src>
            $<TARGET_OBJECTS:notificationmanager_src>

            # utilities
            $<TARGET_OBJECTS:slideshow_src>
            $<TARGET_OBJECTS:imageeditor_src>
            $<TARGET_OBJECTS:metadataedit_src>
            $<TARGET_OBJECTS:expoblending_src>
            $<TARGET_OBJECTS:assistants_src>
            $<TARGET_OBJECTS:presentation_src>
            $<TARGET_OBJECTS:digikamlibtransitionmngr_src>
            $<TARGET_OBJECTS:akonadiiface_src>  # NOTE: even if optional this code need to be compiled here.
            $<TARGET_OBJECTS:calendar_src>
            $<TARGET_OBJECTS:sendbymail_src>
            $<TARGET_OBJECTS:printcreator_src>
            $<TARGET_OBJECTS:mediaserver_src>
            $<TARGET_OBJECTS:webservices_src>

            utils/digikam_debug.cpp
)

if(Marble_FOUND)
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

if(FLEX_FOUND AND BISON_FOUND AND KF5ThreadWeaver_FOUND)
    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:panorama_src>
    )
endif()

if(LibXml2_FOUND AND LibXslt_FOUND)
    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:htmlgallery_src>
    )
endif()

if(ENABLE_MEDIAPLAYER)
    set(DIGIKAMCORE_OBJECTS
        ${DIGIKAMCORE_OBJECTS}
        $<TARGET_OBJECTS:videoslideshow_src>
    )
endif()


include_directories($<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::WindowSystem,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>)

if(KF5KIO_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(HAVE_OPENGL)
    include_directories($<TARGET_PROPERTY:Qt5::OpenGL,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(Gphoto2_FOUND)
    include_directories(${GPHOTO2_INCLUDE_DIRS})
endif()

add_library(digikamcore
            SHARED
            ${DIGIKAMCORE_OBJECTS}
)

add_dependencies(digikamcore digikam-gitversion)

generate_export_header(digikamcore BASE_NAME digikam EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_core_export.h")

target_link_libraries(digikamcore
                      PUBLIC

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Xml
                      Qt5::XmlPatterns
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::WebKitWidgets
                      Qt5::PrintSupport
                      Qt5::Concurrent

                      KF5::Solid
                      KF5::WindowSystem
                      KF5::ConfigGui
                      KF5::CoreAddons
                      KF5::Service
                      KF5::XmlGui
                      KF5::I18n

                      # Required by CImg which use pthread internally.

                      ${CMAKE_THREAD_LIBS_INIT}

                      ${LCMS2_LIBRARIES} # filters

                      ${TIFF_LIBRARIES}
                      PNG::PNG
                      ${EXIV2_LIBRARIES}

                      ${FFMPEG_LIBRARIES}

                      libdng
                      libraw
)

if(ENABLE_DBUS)
    target_link_libraries(digikamcore PUBLIC Qt5::DBus)
endif()

if(KF5IconThemes_FOUND)
    target_link_libraries(digikamcore PUBLIC KF5::IconThemes)
endif()

if(KF5KIO_FOUND)
    target_link_libraries(digikamcore PUBLIC KF5::KIOCore)
    target_link_libraries(digikamcore PUBLIC KF5::KIOWidgets)
endif()

if(KF5Notifications_FOUND)
    target_link_libraries(digikamcore PUBLIC KF5::Notifications)
endif()

if(KF5NotifyConfig_FOUND)
    target_link_libraries(digikamcore PUBLIC KF5::NotifyConfig)
endif()

if(Marble_FOUND)
    target_link_libraries(digikamcore PUBLIC ${MARBLE_LIBRARIES})
endif()

if(X11_FOUND)
    target_link_libraries(digikamcore PUBLIC Qt5::X11Extras ${X11_LIBRARIES})
endif()

if(Jasper_FOUND)
    target_link_libraries(digikamcore PUBLIC ${JASPER_LIBRARIES})
endif()

# LibLqr-1 library rules for content-aware filter
if(Lqr-1_FOUND)
    target_link_libraries(digikamcore PRIVATE ${LQR-1_LIBRARIES})
endif()

if(LensFun_FOUND)
    target_include_directories(digikamcore PUBLIC ${LENSFUN_INCLUDE_DIRS})
    target_link_libraries(digikamcore PUBLIC ${LENSFUN_LIBRARIES})
endif()

# for nrfilter
if(OpenCV_FOUND)
    target_link_libraries(digikamcore PRIVATE ${OpenCV_LIBRARIES})
endif()

if(KF5FileMetaData_FOUND)
    target_link_libraries(digikamcore PUBLIC baloowrap)
endif()

if(HAVE_OPENGL)
    target_link_libraries(digikamcore
                          PUBLIC
                          Qt5::OpenGL
                          ${OPENGL_LIBRARIES}
    )
endif()

if(KF5Sane_FOUND)
    target_link_libraries(digikamcore PUBLIC KF5::Sane)
endif()

if(KF5AkonadiContact_FOUND)
    target_link_libraries(digikamcore PUBLIC KF5::AkonadiContact)
endif()

if(KF5CalendarCore_FOUND)
    target_link_libraries(digikamcore PUBLIC KF5::CalendarCore)
endif()

if(FLEX_FOUND AND BISON_FOUND AND KF5ThreadWeaver_FOUND)
    target_link_libraries(digikamcore PUBLIC KF5::ThreadWeaver)
endif()

if(LibXml2_FOUND AND LibXslt_FOUND)
    target_link_libraries(digikamcore PUBLIC ${LIBXSLT_EXSLT_LIBRARIES}
                                             ${LIBXSLT_LIBRARIES}
                                             ${LIBXML2_LIBRARIES})
endif()

if(KF5MediaWiki_FOUND)
    target_link_libraries(digikamcore PUBLIC ${LIBMEDIAWIKI_LIBRARIES})
endif()

if(KF5Vkontakte_FOUND)
    target_link_libraries(digikamcore PUBLIC ${LIBKVKONTAKTE_LIBRARIES})
endif()

if(APPLE)
    target_link_libraries(digikamcore PUBLIC /System/Library/Frameworks/AppKit.framework)
endif()

if(WIN32)
  target_link_libraries(digikamcore PUBLIC wsock32 ws2_32)
endif()

if(ENABLE_MEDIAPLAYER)
    target_link_libraries(digikamcore PUBLIC ${QTAV_LIBRARIES})
endif()

set_target_properties(digikamcore PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

install(TARGETS digikamcore ${INSTALL_TARGETS_DEFAULT_ARGS})

########################## digiKam GUI sources #############################

if(ENABLE_DBUS)
    qt5_add_dbus_adaptor(digikamadaptor_SRCS main/org.kde.digikam.xml main/digikamapp.h Digikam::DigikamApp)
endif()

set(libdigikamgui_SRCS
    main/digikamapp.cpp

    date/dpopupframe.cpp
    date/ddateedit.cpp
    date/ddatetable.cpp
    date/ddatetable_p.cpp
    date/ddatepicker.cpp
    date/ddatepicker_p.cpp
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
    items/digikamimagefacedelegate.cpp
    items/overlays/assignnameoverlay.cpp
    items/overlays/facerejectionoverlay.cpp
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
    utils/componentsinfo.cpp

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

if(${Marble_FOUND})
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


set(DIGIKAM_OBJECTS
            $<TARGET_OBJECTS:digikamdatabasemain_src>
            $<TARGET_OBJECTS:digikamfacesenginedatabase_src>
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
            $<TARGET_OBJECTS:filters_src>
            $<TARGET_OBJECTS:importuibackend_src>
            $<TARGET_OBJECTS:imagehistorywidgets_src>
            $<TARGET_OBJECTS:iojobs_src>
            $<TARGET_OBJECTS:dtrash_src>
            $<TARGET_OBJECTS:facemanagement_src>
            $<TARGET_OBJECTS:queuemanager_src>
            $<TARGET_OBJECTS:importui_src>
            $<TARGET_OBJECTS:advancedrename_src>
)

if(${Marble_FOUND})
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
                      PUBLIC

                      digikamdatabase
                      digikamcore

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::WebKitWidgets
                      Qt5::PrintSupport

                      KF5::Solid
                      KF5::Service
                      KF5::WindowSystem
                      KF5::I18n

                      ${OpenCV_LIBRARIES}
)

if(ENABLE_DBUS)
    target_link_libraries(digikamgui PUBLIC Qt5::DBus)
endif()

if(KF5IconThemes_FOUND)
    target_link_libraries(digikamgui PUBLIC KF5::IconThemes)
endif()

if(KF5KIO_FOUND)
    target_link_libraries(digikamgui PUBLIC KF5::KIOWidgets)
endif()

if(${Marble_FOUND})
     target_link_libraries(digikamgui PUBLIC ${MARBLE_LIBRARIES})
endif()

if(APPLE)
    target_link_libraries(digikamgui PRIVATE /System/Library/Frameworks/AppKit.framework)
endif()

if(HAVE_OPENGL)
    target_link_libraries(digikamgui
                          PUBLIC
                          Qt5::OpenGL
                          ${OPENGL_LIBRARIES}
    )
endif()

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(digikamgui PUBLIC ${MATH_LIBRARY})
endif()

if(WIN32)
    set_target_properties(digikamgui PROPERTIES COMPILE_FLAGS -DJPEG_STATIC)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(digikamgui PRIVATE ${KVM_LIBRARY})
endif()

if(Gphoto2_FOUND)
    # See bug #258931: libgphoto2 library must be the last arg for linker.
    # See bug #268267 : digiKam need to be linked to libusb to prevent crash 
    # at gphoto2 init if opencv is linked with libdc1394. Libusb linking rules are 
    # add to gphoto2 linking rules by Gphoto2 cmake detection script.

    target_link_libraries(digikamgui PUBLIC ${GPHOTO2_LIBRARIES})

endif()

set_target_properties(digikamgui PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

install(TARGETS digikamgui ${INSTALL_TARGETS_DEFAULT_ARGS})

#################### digiKam Executable  ####################################

set(digikam_SRCS
    main/main.cpp
)

# this is only required by Win & OSX
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../data/icons/apps/*-apps-digikam.png")
ecm_add_app_icon(digikam_SRCS ICONS ${ICONS_SRCS})

add_executable(digikam ${digikam_SRCS})

add_dependencies(digikam digikam-gitversion)

# To fill plist XML file for OSX ############

set(MACOSX_APP_NAME_STRING             "digikam")
set(MACOSX_APP_DESCRIPTION             "Advanced digital photo management application")
set(MACOSX_BUNDLE_LONG_VERSION_STRING  ${DIGIKAM_VERSION_STRING})
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${DIGIKAM_VERSION_SHORT})
set(MACOSX_BUNDLE_BUNDLE_VERSION       ${DIGIKAM_VERSION_STRING})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/Info.plist.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
set_target_properties(digikam PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

target_link_libraries(digikam
                      PUBLIC

                      digikamcore
                      digikamgui

                      libdng

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets

                      KF5::WindowSystem
                      KF5::I18n
)

if(ENABLE_DBUS)
    target_link_libraries(digikam PUBLIC Qt5::DBus)
endif()

if(KF5IconThemes_FOUND)
    target_link_libraries(digikam PUBLIC KF5::IconThemes)
endif()

if(KF5KIO_FOUND)
    target_link_libraries(digikam PUBLIC KF5::KIOWidgets)
endif()

install(TARGETS digikam ${INSTALL_TARGETS_DEFAULT_ARGS})

# Others Files to install #######################################################

install(PROGRAMS main/org.kde.digikam.desktop     DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES    main/org.kde.digikam.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES    main/digikamui5.rc               DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES    main/digikam.notifyrc            DESTINATION ${KNOTIFYRC_INSTALL_DIR})
