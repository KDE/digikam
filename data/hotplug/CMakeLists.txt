#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(digikamhelperdir ${KDE_INSTALL_FULL_DATADIR}/digikam/utils)

#configure_file(digikam-download.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam-download.desktop @ONLY)
#configure_file(digikam-gphoto2-camera.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam-gphoto2-camera.desktop @ONLY)
#configure_file(digikam-mount-and-download.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam-mount-and-download.desktop @ONLY)

configure_file(digikam-opencamera.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/digikam-opencamera.desktop @ONLY)

#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/digikam-download.desktop
#              ${CMAKE_CURRENT_BINARY_DIR}/digikam-gphoto2-camera.desktop
#              ${CMAKE_CURRENT_BINARY_DIR}/digikam-mount-and-download.desktop
#        DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/digikam-opencamera.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions)

if(NOT WIN32)
    install(PROGRAMS digikam-camera DESTINATION ${digikamhelperdir})
endif()
