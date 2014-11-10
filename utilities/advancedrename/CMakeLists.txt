#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
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

KDE4_ADD_UI_FILES(libadvancedrename_SRCS ${libadvancedrename_UI})

KDE4_ADD_LIBRARY(advancedrename STATIC ${libadvancedrename_SRCS})

target_link_libraries(advancedrename
                      digikamdatabase
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                     )
