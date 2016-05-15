# Some useful macros to detect local or system based libraries
#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(DETECT_LIBKIPI MIN_VERSION)

    if (NOT DIGIKAMSC_COMPILE_LIBKIPI)

        message(STATUS "libkipi : search system based library")
        find_package(KF5Kipi ${MIN_VERSION})

        if(KF5Kipi_FOUND)
            set(KF5Kipi_LIBRARIES KF5::Kipi)
            get_target_property(KF5Kipi_INCLUDE_DIRS KF5::Kipi INTERFACE_INCLUDE_DIRECTORIES)
            set(KF5Kipi_FOUND TRUE)
        else()
            set(KF5Kipi_FOUND FALSE)
        endif()

    else()

        message(STATUS "libkipi : use local library from ${CMAKE_SOURCE_DIR}/extra/libkipi/")
        find_file(KF5Kipi_FOUND CMakeLists.txt PATHS ${CMAKE_SOURCE_DIR}/extra/libkipi/)

        if(NOT KF5Kipi_FOUND)
            message(ERROR "libkipi : local library not found")
            set(KF5Kipi_FOUND FALSE)
        else()
            set(KF5Kipi_FOUND TRUE)
        endif()

        set(KF5Kipi_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/extra/libkipi/src ${CMAKE_BINARY_DIR}/extra/libkipi)
        set(KF5Kipi_LIBRARIES KF5Kipi)

    endif()

    message(STATUS "libkipi found         : ${KF5Kipi_FOUND}")
    message(STATUS "libkipi library       : ${KF5Kipi_LIBRARIES}")
    message(STATUS "libkipi includes      : ${KF5Kipi_INCLUDE_DIRS}")

    if(${KF5Kipi_FOUND})

        # Detect libkipi so version used to compile kipi tool to identify if plugin can be loaded in memory by libkipi.
        # This will be used to populate plugin desktop files.

        find_file(KF5KipiConfig_FOUND libkipi_config.h PATHS ${KF5Kipi_INCLUDE_DIRS})
        file(READ "${KF5KipiConfig_FOUND}" KIPI_CONFIG_H_CONTENT)

        string(REGEX REPLACE
               ".*static +const +int +kipi_binary_version += ([^ ;]+).*"
               "\\1"
               KIPI_LIB_SO_CUR_VERSION_FOUND
               "${KIPI_CONFIG_H_CONTENT}"
              )

        set(KIPI_LIB_SO_CUR_VERSION ${KIPI_LIB_SO_CUR_VERSION_FOUND} CACHE STRING "libkipi so version")
        message(STATUS "libkipi SO version  : ${KIPI_LIB_SO_CUR_VERSION}")

    endif()

endmacro()

###########################################################################################################################################"

macro(DETECT_LIBKSANE MIN_VERSION)

    if (NOT DIGIKAMSC_COMPILE_LIBKSANE)

        message(STATUS "libksane : search system based library")
        find_package(KF5Sane ${MIN_VERSION})

        if(KF5Sane_FOUND)
            set(LIBKSANE_LIBRARIES KF5::Sane)
            get_target_property(LIBKSANE_INCLUDES KF5::Sane INTERFACE_INCLUDE_DIRECTORIES)
            set(KF5Sane_FOUND TRUE)
        else()
            set(KF5Sane_FOUND FALSE)
        endif()

    else()

        message(STATUS "libksane : use local library from ${CMAKE_SOURCE_DIR}/extra/libksane/")
        find_file(KF5Sane_FOUND CMakeLists.txt PATHS ${CMAKE_SOURCE_DIR}/extra/libksane/)

        if(NOT KF5Sane_FOUND)
            message(ERROR "libksane : local library not found")
            set(KF5Sane_FOUND FALSE)
        else()
            set(KF5Sane_FOUND TRUE)
        endif()

        set(LIBKSANE_INCLUDES ${CMAKE_SOURCE_DIR}/extra/libksane/src ${CMAKE_BINARY_DIR}/extra/libksane)
        set(LIBKSANE_LIBRARIES KF5Sane)

    endif()

    message(STATUS "libksane found      : ${KF5Sane_FOUND}")
    message(STATUS "libksane library    : ${LIBKSANE_LIBRARIES}")
    message(STATUS "libksane includes   : ${LIBKSANE_INCLUDES}")

endmacro()
