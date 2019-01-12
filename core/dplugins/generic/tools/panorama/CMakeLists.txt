#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2011-2016, Benjamin Girault, <benjamin dot girault at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying PANO_COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ThreadWeaver,INTERFACE_INCLUDE_DIRECTORIES>
)

set(panoramaplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/panotask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/commandtask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/createfinalptotask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/cpfindtask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/cpcleantask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/optimisationtask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/autocroptask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/createmktask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/compilemksteptask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/compilemktask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/huginexecutortask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/copyfilestask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/createpreviewtask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/createptotask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/preprocesstask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/cpfindbinary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/panoactionthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/panomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/panointropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/panoitemspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/panolastpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/panooptimizepage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/panopreprocesspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/panopreviewpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/panowizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/panoramaplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(GENEPanoramaPlugin
                   ${panoramaplugin_SRCS}
)
