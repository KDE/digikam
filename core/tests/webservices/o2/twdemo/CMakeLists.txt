#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================
# Stand alone version of ExpoBlending tool

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
# Twitter demo test tool

set(twdemo_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dplugins/webservices/o2/examples/twitterdemo/tweeter.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dplugins/webservices/o2/examples/twitterdemo/main.cpp
)

add_executable(twdemo ${twdemo_SRCS})

target_link_libraries(twdemo
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

target_compile_definitions(twdemo PRIVATE signals=Q_SIGNALS slots=Q_SLOTS)
