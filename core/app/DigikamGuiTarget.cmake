#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# digiKAm GUI shared library

if(ENABLE_DBUS)
    qt5_add_dbus_adaptor(digikamadaptor_SRCS main/org.kde.digikam.xml main/digikamapp.h Digikam::DigikamApp)
endif()

set(libdigikamgui_SRCS
    main/digikamapp.cpp
    main/digikamapp_solid.cpp
    main/digikamapp_camera.cpp
    main/digikamapp_import.cpp
    main/digikamapp_config.cpp
    main/digikamapp_tools.cpp
    main/digikamapp_setup.cpp

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
    dragdrop/itemdragdrop.cpp
    dragdrop/tagdragdrop.cpp

    filters/filtersidebarwidget.cpp
    filters/tagfilterview.cpp

    items/delegate/digikamitemdelegate.cpp
    items/delegate/itemdelegate.cpp
    items/delegate/itemfacedelegate.cpp
    items/views/digikamitemview.cpp
    items/views/digikamitemview_p.cpp
    items/views/itemcategorizedview.cpp
    items/thumbbar/itemthumbnailbar.cpp
    items/thumbbar/itemthumbnaildelegate.cpp
    items/overlays/assignnameoverlay.cpp
    items/overlays/facerejectionoverlay.cpp
    items/overlays/groupindicatoroverlay.cpp
    items/overlays/itemfullscreenoverlay.cpp
    items/overlays/itemratingoverlay.cpp
    items/overlays/itemrotationoverlay.cpp
    items/overlays/itemcoordinatesoverlay.cpp
    items/overlays/itemselectionoverlay.cpp
    items/utils/itemviewutilities.cpp
    items/utils/tooltipfiller.cpp
    items/utils/contextmenuhelper.cpp
    items/utils/groupingviewimplementation.cpp
    items/utils/itemcategorydrawer.cpp

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

    views/stack/welcomepageview.cpp
    views/stack/itemiconview.cpp
    views/stack/trashview.cpp
    views/stack/stackedview.cpp
    views/preview/itempreviewcanvas.cpp
    views/preview/itempreviewview.cpp
    views/sidebar/leftsidebarwidgets.cpp
    views/sidebar/sidebarwidget.cpp
    views/utils/dmodelfactory.cpp
    views/utils/slideshowbuilder.cpp
    views/utils/componentsinfodlg.cpp

    ${digikamadaptor_SRCS}
)

if(${Marble_FOUND})
    set(libdigikamgui_SRCS
        ${libdigikamgui_SRCS}
        views/stack/mapwidgetview.cpp
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
            $<TARGET_OBJECTS:itempropertiesdigikam_src>
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

set_target_properties(digikamgui PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

if(WIN32)
    set_target_properties(digikamgui PROPERTIES COMPILE_FLAGS -DJPEG_STATIC)
endif()

target_link_libraries(digikamgui
                      PUBLIC

                      digikamdatabase
                      digikamcore

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::PrintSupport

                      KF5::Solid
                      KF5::Service
                      KF5::WindowSystem
                      KF5::I18n

                      ${OpenCV_LIBRARIES}
)

if(ENABLE_QWEBENGINE)
    target_link_libraries(digikamgui PUBLIC Qt5::WebEngineWidgets)
else()
    target_link_libraries(digikamgui PUBLIC Qt5::WebKitWidgets)
endif()

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

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(digikamgui PUBLIC ${MATH_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(digikamcore PRIVATE ${KVM_LIBRARY})
endif()

if(Gphoto2_FOUND)
    # See bug #258931: libgphoto2 library must be the last arg for linker.
    # See bug #268267 : digiKam need to be linked to libusb to prevent crash
    # at gphoto2 init if opencv is linked with libdc1394. Libusb linking rules are
    # add to gphoto2 linking rules by Gphoto2 cmake detection script.

    target_link_libraries(digikamgui PUBLIC ${GPHOTO2_LIBRARIES})

endif()

install(TARGETS digikamgui EXPORT DigikamGuiConfig ${INSTALL_TARGETS_DEFAULT_ARGS})
install(EXPORT DigikamGuiConfig  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/digikam" NAMESPACE Digikam::)
