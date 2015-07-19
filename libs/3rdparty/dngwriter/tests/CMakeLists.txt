#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================
# DNGVALIDATE tool from DNG SDK

set(dngvalidate_SRCS ../extra/dng_sdk/dng_validate.cpp)

add_executable(dngvalidate ${dngvalidate_SRCS})
ecm_mark_nongui_executable(dngvalidate)

target_link_libraries(dngvalidate
                      digikamcore
                      ${CMAKE_THREAD_LIBS_INIT}
                     )

if(${KF5KGeoMap_FOUND})
    target_link_libraries(dngvalidate ${KF5KGeoMap_LIBRARIES})
endif()

# =======================================================
# DNGINFO command line tool

set(dnginfo_SRCS dnginfo.cpp)

add_executable(dnginfo ${dnginfo_SRCS})
ecm_mark_nongui_executable(dnginfo)

target_link_libraries(dnginfo
                      digikamcore
                      Qt5::Core
                      ${EXPAT_LIBRARY}
                      ${CMAKE_THREAD_LIBS_INIT}
                     )

if(${KF5KGeoMap_FOUND})
    target_link_libraries(dnginfo ${KF5KGeoMap_LIBRARIES})
endif()

# =======================================================
# RAW2DNG command line tool

set(raw2dng_SRCS raw2dng.cpp)

add_executable(raw2dng
               ${raw2dng_SRCS}
              )

ecm_mark_nongui_executable(raw2dng)

target_link_libraries(raw2dng
                      digikamcore
                      Qt5::Core
                      KF5::KIOCore
                      KF5::KExiv2
                      KF5::KDcraw
                      ${CMAKE_THREAD_LIBS_INIT}
                     )

if(${KF5KGeoMap_FOUND})
    target_link_libraries(raw2dng ${KF5KGeoMap_LIBRARIES})
endif()
