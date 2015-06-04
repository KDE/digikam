#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# sources
file(GLOB libadvancedrename_SRCS 
          ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/database/keys/*.cpp
    )

# ui files
file(GLOB libadvancedrename_UI 
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/modifiers/*.ui
          ${CMAKE_CURRENT_SOURCE_DIR}/parser/options/*.ui
    )

ki18n_wrap_ui(libadvancedrename_SRCS ${libadvancedrename_UI})

# TODO for internal use, prefer complete include paths instead of this
include_directories(common/ parser/options/ parser/options/database/ parser/options/database/keys/ parser/modifiers/)

add_library(advancedrename STATIC ${libadvancedrename_SRCS})

# TODO importui is not using partial paths, but direct one for from those
target_include_directories(advancedrename PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/common>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/parser>
)

target_link_libraries(advancedrename
                      digikamdatabase
                        
                      Qt5::Widgets
                      Qt5::Sql

                      KF5::I18n # TODO verify if needed
                      
                      KF5::KExiv2
                      KF5::KDcraw
                     )
