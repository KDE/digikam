#
# Copyright (c) 2015,      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
# Copyright (c) 2010-2017, by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

add_executable(metareaderthread metareaderthread.cpp)
ecm_mark_nongui_executable(metareaderthread)
target_link_libraries(metareaderthread
                      digikamcore
                      Qt5::Core
)

set(testTag_SRCS tagsreadwritetest.cpp)
add_executable(tagsReadWriteTest ${testTag_SRCS})
add_test(tagsReadWriteTest tagsReadWriteTest)
ecm_mark_as_test(tagsReadWriteTest)

target_link_libraries(tagsReadWriteTest
                      digikamcore
                      libdng

                      Qt5::Core
                      Qt5::Test

                      KF5::I18n
)

set(testRating_SRCS ratingreadwritetest.cpp)
add_executable(ratingReadWriteTest ${testRating_SRCS})
add_test(ratingReadWriteTest ratingReadWriteTest)
ecm_mark_as_test(ratingReadWriteTest)

target_link_libraries(ratingReadWriteTest
                      digikamcore
                      libdng

                      Qt5::Core
                      Qt5::Test

                      KF5::I18n
)

set(testComment_SRCS commentreadwritetest.cpp)
add_executable(commentReadWriteTest ${testComment_SRCS})
add_test(commentReadWriteTest commentReadWriteTest)
ecm_mark_as_test(commentReadWriteTest)

target_link_libraries(commentReadWriteTest
                      digikamcore
                      libdng

                      Qt5::Core
                      Qt5::Test

                      KF5::I18n
)

set(setiptcpreview_SRCS setiptcpreview.cpp)
add_executable(setiptcpreview ${setiptcpreview_SRCS})
ecm_mark_nongui_executable(setiptcpreview)
target_link_libraries(setiptcpreview
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)

set(loadfromba_SRCS loadfromba.cpp)
add_executable(loadfromba ${loadfromba_SRCS})
ecm_mark_nongui_executable(loadfromba)
target_link_libraries(loadfromba
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)

set(readfromimage_SRCS readfromimage.cpp)
add_executable(readfromimage ${readfromimage_SRCS})
ecm_mark_nongui_executable(readfromimage)
target_link_libraries(readfromimage
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)

set(erasetag_SRCS erasetag.cpp)
add_executable(erasetag ${erasetag_SRCS})
ecm_mark_nongui_executable(erasetag)
target_link_libraries(erasetag
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)

set(printtagslist_SRCS printtagslist.cpp)
add_executable(printtagslist ${printtagslist_SRCS})
ecm_mark_nongui_executable(printtagslist)
target_link_libraries(printtagslist
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)

set(usexmpsidecar_SRCS usexmpsidecar.cpp)
add_executable(usexmpsidecar ${usexmpsidecar_SRCS})
ecm_mark_nongui_executable(usexmpsidecar)
target_link_libraries(usexmpsidecar
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)

set(readimagewritexmpsidecar_SRCS readimagewritexmpsidecar.cpp)
add_executable(readimagewritexmpsidecar ${readimagewritexmpsidecar_SRCS})
ecm_mark_nongui_executable(readimagewritexmpsidecar)
target_link_libraries(readimagewritexmpsidecar
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)

set(setxmpface_SRCS setxmpface.cpp)
add_executable(setxmpface ${setxmpface_SRCS})
ecm_mark_nongui_executable(setxmpface)
target_link_libraries(setxmpface
                      digikamcore
                      libdng

                      Qt5::Core

                      KF5::I18n
)
