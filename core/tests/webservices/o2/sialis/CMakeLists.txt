#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================
# Stand alone version of ExpoBlending tool

include_directories(
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Qml,INTERFACE_INCLUDE_DIRECTORIES>
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

set(sialis_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dplugins/webservices/o2/examples/sialis/tweetmodel.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dplugins/webservices/o2/examples/sialis/twitterapi.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dplugins/webservices/o2/examples/sialis/main.cpp
)

qt5_add_resources(QT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs/dplugins/webservices/o2/examples/sialis/main.qml
)

add_executable(sialis ${sialis_SRCS})

target_link_libraries(sialis
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
                      Qt5::Qml
                      Qt5::WebView
)

target_compile_definitions(sialis PRIVATE signals=Q_SIGNALS slots=Q_SLOTS)
