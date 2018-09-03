#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2011-2015, Benjamin Girault, <benjamin dot girault at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================
# Stand alone version of ExpoBlending tool

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

macro(ASSISTANTS_BUILD_STANDALONE)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target)

    set(${_target}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${_filename})

    add_executable(${_target} ${${_target}_SRCS})

    target_link_libraries(${_target}
                          digikamcore
                          Qt5::Gui
                          Qt5::Xml
    )

endmacro()

ASSISTANTS_BUILD_STANDALONE(expoblending)
ASSISTANTS_BUILD_STANDALONE(calendar)
ASSISTANTS_BUILD_STANDALONE(sendbymail)
ASSISTANTS_BUILD_STANDALONE(printcreator)

if(FLEX_FOUND AND BISON_FOUND AND KF5ThreadWeaver_FOUND)
    ASSISTANTS_BUILD_STANDALONE(panorama)
    ASSISTANTS_BUILD_STANDALONE(panoparser_test)
endif()

if(ENABLE_MEDIAPLAYER)
    ASSISTANTS_BUILD_STANDALONE(videoslideshow)
endif()

if(LibXml2_FOUND AND LibXslt_FOUND)
    ASSISTANTS_BUILD_STANDALONE(htmlgallery)
endif()
