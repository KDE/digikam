#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

file(GLOB libadvancedrename_SRCS
          ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys/*.cpp
)

file(GLOB libadvancedrename_UI
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/*.ui
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/*.ui
)

ki18n_wrap_ui(libadvancedrename_SRCS ${libadvancedrename_UI})

# TODO for internal use, prefer complete include paths instead of this
include_directories(common/ parser/options/ parser/options/database/ parser/options/database/keys/ parser/modifiers/)

include_directories(
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(advancedrename_src OBJECT ${libadvancedrename_SRCS})
