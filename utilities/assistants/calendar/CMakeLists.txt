#
# Copyright (c) 2010-2016, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5CalendarCore_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::CalendarCore,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

set(libcalendar_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/print/calsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print/calpainter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print/calprinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print/calsystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/calintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/calmonthwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/caltemplate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/calwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/calwizard.cpp
)

ki18n_wrap_ui(libcalendar_SRCS
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/caltemplate.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/calevents.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/calprogress.ui
)

add_library(calendar_src OBJECT ${libcalendar_SRCS})
