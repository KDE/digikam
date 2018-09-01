# Some useful macros to detect local or system based libraries
#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(DETECT_LIBKVKONTAKTE MIN_VERSION)

    if (NOT DIGIKAMSC_COMPILE_LIBKVKONTAKTE)

        message(STATUS "libkvkontakte : search system based library")
        find_package(KF5Vkontakte ${MIN_VERSION} QUIET)

        if(KF5Vkontakte_FOUND)
            set(LIBKVKONTAKTE_LIBRARIES KF5::Vkontakte)
            get_target_property(LIBKVKONTAKTE_INCLUDES KF5::Vkontakte INTERFACE_INCLUDE_DIRECTORIES)
            set(KF5Vkontakte_FOUND TRUE)
        else()
            set(KF5Vkontakte_FOUND FALSE)
        endif()

    else()

        message(STATUS "libkvkontakte : use local library from ${CMAKE_SOURCE_DIR}/extra/libkvkontakte/")

        if(EXISTS "${CMAKE_SOURCE_DIR}/extra/libkvkontakte/CMakeLists.txt")
            set(KF5Vkontakte_FOUND TRUE)

            # Create symbolic links to compile fine with client codes.
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
                            ${CMAKE_SOURCE_DIR}/extra/libkvkontakte/src/
                            ${CMAKE_SOURCE_DIR}/extra/libkvkontakte/Vkontakte)
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
                            ${CMAKE_BINARY_DIR}/extra/libkvkontakte/src/
                            ${CMAKE_BINARY_DIR}/extra/libkvkontakte/Vkontakte)

        else()
            message(WARNING "libkvkontakte : local library not found")
            set(KF5Vkontakte_FOUND FALSE)
        endif()

        set(LIBKVKONTAKTE_INCLUDES ${CMAKE_SOURCE_DIR}/extra/libkvkontakte/
                                   ${CMAKE_BINARY_DIR}/extra/libkvkontakte/
                                   ${CMAKE_BINARY_DIR}/extra/libkvkontakte/Vkontakte)
        set(LIBKVKONTAKTE_LIBRARIES KF5Vkontakte)

    endif()

    message(STATUS "libkvkontakte found      : ${KF5Vkontakte_FOUND}")
    message(STATUS "libkvkontakte library    : ${LIBKVKONTAKTE_LIBRARIES}")
    message(STATUS "libkvkontakte includes   : ${LIBKVKONTAKTE_INCLUDES}")

endmacro()
