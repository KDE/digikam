#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# We need to include the setup dir for showfoto BEFORE the other include dirs,
# otherwise the one from the "utilities" dir will be used
include_directories(BEFORE
                    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar
                    ${CMAKE_CURRENT_SOURCE_DIR}/setup
                    ${CMAKE_CURRENT_SOURCE_DIR}/main
)

#Require by Greycstoration algorithm (CImg.h)
kde_enable_exceptions()

set(showfoto_SRCS
    ${libsetupshowfoto_SRCS}
    ${libshowfotosetup_SRCS}

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

KDE4_ADD_APP_ICON(showfoto_SRCS "../data/icons/apps/hi*-app-showfoto.png")

add_executable(showfoto ${showfoto_SRCS})

# To fill plist XML file for OSX #####################################################

set(MACOSX_APP_NAME_STRING             "showfoto")
set(MACOSX_APP_DESCRIPTION             "Stand alone Photo Editor based on digiKam Image Editor")
set(MACOSX_BUNDLE_LONG_VERSION_STRING  ${DIGIKAM_VERSION_STRING})
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${DIGIKAM_VERSION_SHORT})
set(MACOSX_BUNDLE_BUNDLE_VERSION       ${DIGIKAM_VERSION_STRING})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/templates/Info.plist.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
set_target_properties(showfoto PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

target_link_libraries(showfoto
                      digikamcore

                      ${KF5KExiv2_LIBRARIES}
                      ${KF5KDcraw_LIBRARIES}
                      ${KGEOMAP_LIBRARIES}
                      KF5::KIOCore
                      ${LCMS_LIBRARIES}
                      ${OpenCV_LIBRARIES}
)

if(${KF5KGeoMap_FOUND})
   target_link_libraries(showfoto ${KGEOMAP_LIBRARIES})
endif()

if(NOT WIN32)
    # To link under Solaris (see bug #274484)
    target_link_libraries(showfoto ${MATH_LIBRARY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    target_link_libraries(showfoto ${KVM_LIBRARY})
endif()

install(TARGETS  showfoto ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS main/showfoto.desktop     DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES    main/showfoto.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata)
install(FILES    main/showfotoui.rc        DESTINATION ${KXMLGUI_INSTALL_DIR}/showfoto)
