# Some useful macros to detect local or system based libraries
#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(DETECT_LIBKSANE MIN_VERSION)

    if (NOT DIGIKAMSC_COMPILE_LIBKSANE)

        message(STATUS "libksane : search system based library")
        find_package(KF5Sane ${MIN_VERSION} QUIET)

        if(KF5Sane_FOUND)
            set(LIBKSANE_LIBRARIES KF5::Sane)
            get_target_property(LIBKSANE_INCLUDES KF5::Sane INTERFACE_INCLUDE_DIRECTORIES)
            set(KF5Sane_FOUND TRUE)
        else()
            set(KF5Sane_FOUND FALSE)
        endif()

    else()

        message(STATUS "libksane : use local library from ${CMAKE_SOURCE_DIR}/extra/libksane/")

        if(EXISTS "${CMAKE_SOURCE_DIR}/extra/libksane/CMakeLists.txt")
            set(KF5Sane_FOUND TRUE)
        else()
            message(WARNING "libksane : local library not found")
            set(KF5Sane_FOUND FALSE)
        endif()

        set(LIBKSANE_INCLUDES ${CMAKE_SOURCE_DIR}/extra/libksane/src ${CMAKE_BINARY_DIR}/extra/libksane)
        set(LIBKSANE_LIBRARIES KF5Sane)

    endif()

    message(STATUS "libksane found      : ${KF5Sane_FOUND}")
    message(STATUS "libksane library    : ${LIBKSANE_LIBRARIES}")
    message(STATUS "libksane includes   : ${LIBKSANE_INCLUDES}")

endmacro()

###########################################################################################################################################"

macro(DETECT_LIBMEDIAWIKI MIN_VERSION)

    if (NOT DIGIKAMSC_COMPILE_LIBMEDIAWIKI)

        message(STATUS "libmediawiki : search system based library")
        find_package(KF5MediaWiki ${MIN_VERSION} QUIET)

        if(KF5MediaWiki_FOUND)
            set(LIBMEDIAWIKI_LIBRARIES KF5::MediaWiki)
            get_target_property(LIBMEDIAWIKI_INCLUDES KF5::MediaWiki INTERFACE_INCLUDE_DIRECTORIES)
            set(KF5MediaWiki_FOUND TRUE)
        else()
            set(KF5MediaWiki_FOUND FALSE)
        endif()

    else()

        message(STATUS "libmediawiki : use local library from ${CMAKE_SOURCE_DIR}/extra/libmediawiki/")

        if(EXISTS "${CMAKE_SOURCE_DIR}/extra/libmediawiki/CMakeLists.txt")
            set(KF5MediaWiki_FOUND TRUE)
        else()
            message(WARNING "libmediawiki : local library not found")
            set(KF5MediaWiki_FOUND FALSE)
        endif()

        set(LIBMEDIAWIKI_INCLUDES ${CMAKE_SOURCE_DIR}/extra/libmediawiki/ ${CMAKE_BINARY_DIR}/extra/libmediawiki)
        set(LIBMEDIAWIKI_LIBRARIES KF5MediaWiki)

    endif()

    message(STATUS "libmediawiki found      : ${KF5MediaWiki_FOUND}")
    message(STATUS "libmediawiki library    : ${LIBMEDIAWIKI_LIBRARIES}")
    message(STATUS "libmediawiki includes   : ${LIBMEDIAWIKI_INCLUDES}")

endmacro()

###########################################################################################################################################"

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
        else()
            message(WARNING "libkvkontakte : local library not found")
            set(KF5Vkontakte_FOUND FALSE)
        endif()

        set(LIBKVKONTAKTE_INCLUDES ${CMAKE_SOURCE_DIR}/extra/libkvkontakte/ ${CMAKE_BINARY_DIR}/extra/libkvkontakte)
        set(LIBKVKONTAKTE_LIBRARIES KF5Vkontakte)

    endif()

    message(STATUS "libkvkontakte found      : ${KF5Vkontakte_FOUND}")
    message(STATUS "libkvkontakte library    : ${LIBKVKONTAKTE_LIBRARIES}")
    message(STATUS "libkvkontakte includes   : ${LIBKVKONTAKTE_INCLUDES}")

endmacro()
