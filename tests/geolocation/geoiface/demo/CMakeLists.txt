#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(DETECT_LIBKEXIV2 MIN_VERSION)

    if (NOT DIGIKAMSC_COMPILE_LIBKEXIV2)

        message(STATUS "libkexiv2 : search system based library")
        find_package(KF5KExiv2 ${MIN_VERSION})

        if(KF5KExiv2_FOUND)
            set(KF5KExiv2_LIBRARIES KF5::KExiv2)
        else()
            set(KF5KExiv2_FOUND FALSE)
        endif()

    else()

        message(STATUS "libkexiv2 : use local library from ${CMAKE_SOURCE_DIR}/extra/libkexiv2/")
        find_file(KF5KExiv2_FOUND CMakeLists.txt PATHS ${CMAKE_SOURCE_DIR}/extra/libkexiv2/)

        if(NOT KF5KExiv2_FOUND)
            message(ERROR "libkexiv2 : local library not found")
        endif()

        set(KF5KExiv2_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/extra/libkexiv2/src ${CMAKE_BINARY_DIR}/extra/libkexiv2/src ${CMAKE_BINARY_DIR}/extra/libkexiv2)
        set(KF5KExiv2_LIBRARIES KF5KExiv2)

    endif()

    message(STATUS "libkexiv2 found       : ${KF5KExiv2_FOUND}")
    message(STATUS "libkexiv2 include dir : ${KF5KExiv2_INCLUDE_DIRS}")
    message(STATUS "libkexiv2 library     : ${KF5KExiv2_LIBRARIES}")

endmacro()

DETECT_LIBKEXIV2("5.0.0")

if(KF5KExiv2_FOUND)

    message(STATUS "KExiv2 shared lib found, the demo application will be compiled.")

    include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/../)
    include_directories(${KF5KExiv2_INCLUDE_DIRS})

    set(demo_SRCS
        demo-main.cpp
        myimageitem.cpp
        mytreewidget.cpp
        dragdrophandler.cpp
        mainwindow.cpp
        ../src/kgeomap_common.cpp
        ../src/libkgeomap_debug.cpp
    )

    add_executable(libkgeomap_demo ${demo_SRCS})

    target_link_libraries(libkgeomap_demo
        KF5KGeoMap
        ${KF5KExiv2_LIBRARIES}

        Qt5::Gui
        Qt5::Concurrent

        KF5::ConfigCore
        KF5::CoreAddons
        KF5::I18n
    )

    install(TARGETS libkgeomap_demo ${INSTALL_TARGETS_DEFAULT_ARGS})

else()

    message(STATUS "KExiv2 shared lib NOT found, the demo application will NOT be compiled.")

endif()
