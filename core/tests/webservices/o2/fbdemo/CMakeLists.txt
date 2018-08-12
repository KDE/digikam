#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
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
)


# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY)

# =======================================================
# Facebook demo test tool

set(fbdemo_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/assistants/webservices/common/o2/examples/facebookdemo/fbdemo.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../../utilities/assistants/webservices/common/o2/examples/facebookdemo/main.cpp
)

add_executable(fbdemo ${fbdemo_SRCS})

target_link_libraries(fbdemo
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

target_compile_definitions(fbdemo PRIVATE signals=Q_SIGNALS slots=Q_SLOTS)

