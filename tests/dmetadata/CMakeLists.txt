#
# Copyright (c) 2015 by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include_directories(
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(testTag_SRCS tagsreadwritetest.cpp)
add_executable(tagsReadWriteTest ${testTag_SRCS})
ecm_mark_nongui_executable(tagsReadWriteTest)
target_link_libraries(tagsReadWriteTest
                        digikamcore

                        Qt5::Core
                        Qt5::Test

                        KF5::KExiv2
                        KF5::I18n
                     )

set(testRating_SRCS ratingreadwritetest.cpp)
add_executable(ratingReadWriteTest ${testRating_SRCS})
ecm_mark_nongui_executable(ratingReadWriteTest)
target_link_libraries(ratingReadWriteTest
                        digikamcore

                        Qt5::Core
                        Qt5::Test

                        KF5::KExiv2
                        KF5::I18n
                     )

set(testComment_SRCS commentreadwritetest.cpp)
add_executable(commentReadWriteTest ${testComment_SRCS})
ecm_mark_nongui_executable(commentReadWriteTest)
target_link_libraries(commentReadWriteTest
                        digikamcore
                        Qt5::Core
                        Qt5::Test

                        KF5::KExiv2
                        KF5::I18n
                     )
