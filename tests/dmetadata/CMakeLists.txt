#
# Copyright (c) 2015,      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
# Copyright (c) 2010-2018, by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

macro(METADATA_TESTS_BUILD)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target) 

    set(${_target}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${_target}.cpp)

    add_executable(${_target} ${${_target}_SRCS})
    ecm_mark_nongui_executable(${target})

    target_link_libraries(${_target}
                          digikamcore
                          Qt5::Core
                          KF5::I18n
    )

ENDMACRO()

# ----------------------------------------------------------------------------------

METADATA_TESTS_BUILD(setiptcpreview.cpp)
METADATA_TESTS_BUILD(metareaderthread.cpp)
METADATA_TESTS_BUILD(loadfromba.cpp)
METADATA_TESTS_BUILD(loadfromfile_info.cpp)
METADATA_TESTS_BUILD(loadfromfile_exif.cpp)
METADATA_TESTS_BUILD(loadfromfile_iptc.cpp)
METADATA_TESTS_BUILD(loadfromfile_xmp.cpp)
METADATA_TESTS_BUILD(erasetag.cpp)
METADATA_TESTS_BUILD(printtagslist.cpp)
METADATA_TESTS_BUILD(usexmpsidecar.cpp)
METADATA_TESTS_BUILD(readimagewritexmpsidecar.cpp)
METADATA_TESTS_BUILD(setxmpface.cpp)

# ----------------------------------------------------------------------------------


set(testTag_SRCS tagsreadwritetest.cpp)
add_executable(tagsReadWriteTest ${testTag_SRCS})
add_test(tagsReadWriteTest tagsReadWriteTest)
ecm_mark_as_test(tagsReadWriteTest)

target_link_libraries(tagsReadWriteTest
                      digikamcore
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
                      Qt5::Core
                      Qt5::Test
                      KF5::I18n
)
