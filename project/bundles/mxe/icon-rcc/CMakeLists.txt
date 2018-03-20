#
# Copyright (c) 2010-2017 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

cmake_minimum_required(VERSION 3.0.0)

find_package(Qt5 NO_MODULE REQUIRED Core)
get_target_property(QT_RCC_EXECUTABLE Qt5::rcc LOCATION)

add_executable(qrcAlias main.cpp)
target_link_libraries(qrcAlias PUBLIC Qt5::Core)

add_custom_command(OUTPUT breeze.rcc
    COMMAND ${QT_RCC_EXECUTABLE} -project -o icons.qrc
    COMMAND $<TARGET_FILE_DIR:qrcAlias>/$<TARGET_FILE_NAME:qrcAlias> -i icons.qrc -o aliased.qrc
    COMMAND ${QT_RCC_EXECUTABLE} -binary -o ${CMAKE_BINARY_DIR}/breeze.rcc -root /icons aliased.qrc

    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin/data/icons
    DEPENDS qrcAlias
)

add_custom_command(OUTPUT breeze-dark.rcc
    COMMAND ${QT_RCC_EXECUTABLE} -project -o icons.qrc
    COMMAND $<TARGET_FILE_DIR:qrcAlias>/$<TARGET_FILE_NAME:qrcAlias> -i icons.qrc -o aliased.qrc
    COMMAND ${QT_RCC_EXECUTABLE} -binary -o ${CMAKE_BINARY_DIR}/breeze-dark.rcc -root /icons aliased.qrc

    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin/data/icons
    DEPENDS qrcAlias
)

add_custom_target(icons ALL
    DEPENDS breeze.rcc breeze-dark.rcc
)

install(FILES ${CMAKE_BINARY_DIR}/breeze.rcc      DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
install(FILES ${CMAKE_BINARY_DIR}/breeze-dark.rcc DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
