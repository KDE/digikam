#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(testautocrop_SRCS testautocrop.cpp)
add_executable(testautocrop ${testautocrop_SRCS})
ecm_mark_nongui_executable(testautocrop)

target_link_libraries(testautocrop

                      digikamcore
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::XmlGui

                      KF5::KExiv2
                      KF5::KDcraw

                      ${OpenCV_LIBRARIES}
)

if(MARBLE_FOUND)
    target_link_libraries(testautocrop ${MARBLE_LIBRARIES})
endif()

#------------------------------------------------------------------------

if(LENSFUN_FOUND)

    set(testlensfuniface_SRCS testlensfuniface.cpp)
    add_executable(testlensfuniface ${testlensfuniface_SRCS})
    ecm_mark_nongui_executable(testlensfuniface)

    target_link_libraries(testlensfuniface

                      digikamcore

                      Qt5::Core

                      KF5::I18n
                      KF5::KIOCore
                      KF5::KExiv2

                      ${LENSFUN_LIBRARIES}
                      ${OpenCV_LIBRARIES}
    )

    if(NOT WIN32)
        # To link under Solaris (see bug #274484)
        target_link_libraries(testlensfuniface ${MATH_LIBRARY})
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
        target_link_libraries(testlensfuniface ${KVM_LIBRARY})
    endif()

    if(MARBLE_FOUND)
        target_link_libraries(testlensfuniface ${MARBLE_LIBRARIES})
    endif()

endif()

#------------------------------------------------------------------------

set(testnrestimate_SRCS testnrestimate.cpp)
add_executable (testnrestimate ${testnrestimate_SRCS})
ecm_mark_nongui_executable(testnrestimate)

target_link_libraries(testnrestimate

                      digikamcore
                      digikamdatabase
                      digikamcore

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Test

                      KF5::I18n
                      KF5::KIOCore
                      KF5::KExiv2
                      KF5::KDcraw

                      ${OpenCV_LIBRARIES}
)

if(MARBLE_FOUND)
    target_link_libraries(testnrestimate ${MARBLE_LIBRARIES})
endif()
