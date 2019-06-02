#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroDPlugins)

if(NOT MSVC)
    # Under MacOS 10.14 (Mojave), OpenGL API start to be deprecated in favor to Apple Metal API.
    # https://appleinsider.com/articles/18/06/28/why-macos-mojave-requires-metal----and-deprecates-opengl
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::OpenGL,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(glviewerplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewerplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewerhelpdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewertexture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewertimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glviewerwidget.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    GLViewer
                           SOURCES ${glviewerplugin_SRCS}
                           DEPENDS Qt5::OpenGL
                                   ${OPENGL_LIBRARIES}
)
