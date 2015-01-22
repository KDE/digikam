#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/utils/digikam_version.h.cmake.in    ${CMAKE_CURRENT_BINARY_DIR}/utils/digikam_version.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/database/dbconfigversion.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/database/dbconfigversion.h)

# Shared libdigikamcore ################################################################

set(digikamcore_LIB_SRCS
    # basic libs
#    ${libdimg_SRCS}
#    ${libdimgloaders_SRCS}
#    ${libdimgfilters_SRCS}
#    ${libdmetadata_SRCS}
#    ${libjpegutils_SRCS}
#    ${libprogressmanager_SRCS}
#    ${libdatabasecore_SRCS}
#    ${libthreadimageio_SRCS}
#    ${pgfutils_SRCS}
    ${libdigikamhelpers_SRCS}
#    ${libtthread_SRCS}
#    ${libversionmanager_SRCS}
    ${libkde3rdparty_SRCS}

    # widgets and dialogs
    ${libcolorchooserwidgets_SRCS}
#    ${libcommonwidgets_SRCS}
#    ${libnotificationmanager_SRCS}
#    ${libmainview_SRCS}
#    ${libitemview_SRCS}
#    ${libeditorwidgets_SRCS}
#    ${libiccprofileswidgets_SRCS}
#    ${libmetadatawidgets_SRCS}
#    ${libdialogs_SRCS}
#    ${libimageproperties_SRCS}
#    ${libgraphicsviewwidgets_SRCS}
#    ${libgenericmodels_SRCS}

    # utilities
#    ${libslideshow_SRCS}
#    ${libeditorcore_SRCS}
#    ${libeditordlg_SRCS}
#    ${libeditorprintiface_SRCS}
#    ${libeditorrawimport_SRCS}
#    ${libeditoriface_SRCS}
#    ${libeditorplugin_SRCS}
)


ki18n_wrap_ui(digikamcore_LIB_SRCS
              ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/printiface/printoptionspage.ui
)

kconfig_add_kcfg_files(digikamcore_LIB_SRCS
                       ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/printiface/digikamconfig.kcfgc
)

add_library(digikamcore STATIC ${digikamcore_LIB_SRCS})

add_dependencies(digikamcore digikam-gitversion)

target_link_libraries(digikamcore
                      #digikammodels
                      #digikamdatabase
                      #digikamwidgets
                      
                      #digikamdialogs
                      #digikamtemplate
                      
                      #threadimageio
                      
                      #dthread
                      #dmetadata
                      #dimg
                      
                      imageeditor
                      
                      Qt5::Core
                      Qt5::Sql
                      Qt5::Concurrent

                      KF5::KDELibs4Support
                      KF5::NotifyConfig

                      ${OpenCV_LIBRARIES}
                      ${PGF_LIBRARIES}

                      # Required by CImg library which is using pthread internally.
                      ${CMAKE_THREAD_LIBS_INIT}

                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
)

if(ENABLE_MEDIAPLAYER)
    target_link_libraries(digikamcore Qt5::Multimedia)
endif()

#if(X11_FOUND)
#    target_link_libraries(digikamcore Qt5::X11Extras ${X11_LIBRARIES})
#endif()

if(APPLE)
    target_link_libraries(digikamcore /System/Library/Frameworks/AppKit.framework)
endif()

# For Contents Aware Resizer and Lens auto-correction tools.
if(GLIB2_FOUND)
    target_link_libraries(digikamcore ${GLIB2_LIBRARIES})
endif()

#if(JASPER_FOUND)
#    target_link_libraries(digikamcore ${JASPER_LIBRARIES})
#endif()

if(LENSFUN_FOUND)
   target_link_libraries(digikamcore ${LENSFUN_LIBRARIES})
endif()

if(LQR-1_FOUND)
   target_link_libraries(digikamcore ${liblqr_LIBS})
endif()

if(KF5Kipi_FOUND)
   target_link_libraries(digikamcore ${KF5Kipi_LIBRARIES})
endif()


set_target_properties(digikamcore PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

if(WIN32)
    set_target_properties(digikamcore PROPERTIES COMPILE_FLAGS -DJPEG_STATIC)
    set_target_properties(digikamcore PROPERTIES OUTPUT_NAME   digikamlib)
endif()

install(TARGETS digikamcore ${INSTALL_TARGETS_DEFAULT_ARGS})


# digiKam binary progam #########################################################

set(digikam_SRCS
                 ${libimageversionssidebar_SRCS}
                 
                 main/main.cpp
)

if(KF5Kipi_FOUND)
    set(digikam_SRCS ${digikam_SRCS} ${libkipiiface_SRCS})
endif()

if(KF5KFace_FOUND)
    set(digikam_SRCS ${digikam_SRCS} ${libfacemanagement_SRCS})
endif()


qt5_add_dbus_adaptor(digikam_SRCS main/org.kde.digikam.xml main/digikamapp.h Digikam::DigikamApp)

KDE4_ADD_APP_ICON(digikam_SRCS "../data/icons/apps/hi*-app-digikam.png")

add_executable(digikam ${digikam_SRCS})

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
                      digikammodels

                      progressmanager
                      notificationmanager
                      maintenance
                      slideshow
                      imageeditor
                      fuzzysearch
                      imageeditor

                      kipiiface


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


if(KF5KFace_FOUND)
   target_link_libraries(digikam ${KF5KFace_LIBRARIES})
endif()

if(KF5KGeoMap_FOUND)
   target_link_libraries(digikam ${KF5KGeoMap_LIBRARIES})
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
endif()

if(NOT WIN32 AND GPHOTO2_FOUND)
    # See bug #268267 : digiKam need to be linked to libusb to prevent crash at gphoto2 init if opencv is linked with libdc1394
    target_link_libraries(digikam ${LIBUSB_LIBRARIES})
endif()

install(TARGETS digikam ${INSTALL_TARGETS_DEFAULT_ARGS})

# Others Files to install #######################################################

install(PROGRAMS main/digikam.desktop     DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES    main/digikam.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)
install(FILES    main/digikamui.rc        DESTINATION ${KXMLGUI_INSTALL_DIR}/digikam)
install(FILES    main/digikam.notifyrc    DESTINATION ${KNOTIFYRC_INSTALL_DIR})
