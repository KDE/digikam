#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET_DEBUG_AREA_CODE(${AREA_CODE_SHOWFOTO})

# We need to include the setup dir for showfoto BEFORE the other include dirs,
# otherwise the one from the "utilities" dir will be used
include_directories(BEFORE
                    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar
                    ${CMAKE_CURRENT_SOURCE_DIR}/setup
                    ${CMAKE_CURRENT_SOURCE_DIR}/main
                   )

#Require by Greycstoration algorithm (CImg.h)
if(NOT MSVC)
    add_definitions(${KDE4_ENABLE_EXCEPTIONS})
endif()

set(showfoto_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupiofiles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupicc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup/setupslideshow.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetuptooltip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetupmisc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetupmetadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/setup/showfotosetup.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotoiteminfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotoimagemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotothumbnailmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotofiltermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotodragdrophandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotokineticscroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotoitemsortsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotocoordinatesoverlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/itemviewshowfotodelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotodelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfototooltipfiller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotocategorizedview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thumbbar/showfotothumbnailbar.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfotosettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main/showfoto.cpp
   )

KDE4_ADD_APP_ICON(showfoto_SRCS "../data/icons/apps/hi*-app-showfoto.png")

KDE4_ADD_EXECUTABLE(showfoto ${showfoto_SRCS})

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

                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${KGEOMAP_LIBRARIES}
                      ${KDE4_KIO_LIBS}
                      ${LCMS_LIBRARIES}
                      ${OpenCV_LIBRARIES}
                     )

if(KGEOMAP_FOUND)
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
install(FILES    main/showfotoui.rc        DESTINATION ${DATA_INSTALL_DIR}/showfoto)
