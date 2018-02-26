#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

# We need to include the setup dir for showfoto BEFORE the other include dirs,
# otherwise the one from the "utilities" dir will be used
include_directories(BEFORE
                    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar
                    ${CMAKE_CURRENT_SOURCE_DIR}/setup
                    ${CMAKE_CURRENT_SOURCE_DIR}/main
)

include_directories(
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(showfoto_SRCS
    setup/showfotosetuptooltip.cpp
    setup/showfotosetupmisc.cpp
    setup/showfotosetupmetadata.cpp
    setup/showfotosetupraw.cpp
    setup/showfotosetup.cpp

    thumbbar/showfotoiteminfo.cpp
    thumbbar/showfotoimagemodel.cpp
    thumbbar/showfotothumbnailmodel.cpp
    thumbbar/showfotofiltermodel.cpp
    thumbbar/showfotodragdrophandler.cpp
    thumbbar/showfotokineticscroller.cpp
    thumbbar/showfotoitemsortsettings.cpp
    thumbbar/showfotocoordinatesoverlay.cpp
    thumbbar/itemviewshowfotodelegate.cpp
    thumbbar/showfotodelegate.cpp
    thumbbar/showfototooltipfiller.cpp
    thumbbar/showfotocategorizedview.cpp
    thumbbar/showfotothumbnailbar.cpp

    main/showfotosettings.cpp
    main/main.cpp
    main/showfoto.cpp
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../data/icons/apps/*-apps-showfoto.png")
ecm_add_app_icon(showfoto_SRCS ICONS ${ICONS_SRCS})

add_executable(showfoto ${showfoto_SRCS}
                        $<TARGET_OBJECTS:setupshowfoto_src>
                        $<TARGET_OBJECTS:notificationmanager_src>
)

add_dependencies(showfoto digikam-gitversion)

# To fill plist XML file for OSX #####################################################

set(MACOSX_APP_NAME_STRING             "showfoto")
set(MACOSX_APP_DESCRIPTION             "Stand alone Photo Editor based on digiKam Image Editor")
set(MACOSX_BUNDLE_LONG_VERSION_STRING  ${DIGIKAM_VERSION_STRING})
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${DIGIKAM_VERSION_SHORT})
set(MACOSX_BUNDLE_BUNDLE_VERSION       ${DIGIKAM_VERSION_STRING})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/Info.plist.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
set_target_properties(showfoto PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

target_link_libraries(showfoto
                      PRIVATE
                      digikamcore

                      Qt5::Core

                      KF5::I18n
                      KF5::XmlGui
)

if(KF5Notifications_FOUND)
    target_link_libraries(showfoto PUBLIC KF5::Notifications)
endif()

if(KF5NotifyConfig_FOUND)
    target_link_libraries(showfoto PUBLIC KF5::NotifyConfig)
endif()

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(showfoto PUBLIC ${MATH_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(showfoto PUBLIC ${KVM_LIBRARY})
endif()

install(TARGETS  showfoto                          ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS main/org.kde.showfoto.desktop     DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES    main/org.kde.showfoto.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES    main/showfotoui5.rc               DESTINATION ${KXMLGUI_INSTALL_DIR}/showfoto)
