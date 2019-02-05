#
# Copyright (c) 2015-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>

                    ${LENSFUN_INCLUDE_DIRS}
)

set(lensautofixplugin_SRCS
     ${CMAKE_CURRENT_SOURCE_DIR}/lensautofixplugin.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/lensautofix.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(NAME    LensAutoFix
                                SOURCES ${lensautofixplugin_SRCS}
)
