#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================
# Stand alone version of ExpoBlending tool

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

add_subdirectory(o2)

include_directories(
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

macro(WEBSERVICE_BUILD_STANDALONE)

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

WEBSERVICE_BUILD_STANDALONE(flickr.cpp)
WEBSERVICE_BUILD_STANDALONE(smugmug.cpp)
WEBSERVICE_BUILD_STANDALONE(facebook.cpp)
WEBSERVICE_BUILD_STANDALONE(gdrive.cpp)
WEBSERVICE_BUILD_STANDALONE(gphoto.cpp)
#WEBSERVICE_BUILD_STANDALONE(webservices.cpp)

if(KF5KIO_FOUND)
    WEBSERVICE_BUILD_STANDALONE(filetransfer.cpp)
endif()
