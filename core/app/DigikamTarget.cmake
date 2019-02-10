#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# digiKam executable

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
