#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

add_subdirectory(examples)

include(ECMMarkAsTest)

macro(mediawiki_executable_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp fakeserver/fakeserver.cpp)
        target_link_libraries(${_testname} Qt5::Core Qt5::Test Qt5::Network digikamcore)
        ecm_mark_as_test(${_testname})
        add_test(libmediawiki-${_testname} ${_testname})
    endforeach()
endmacro()

mediawiki_executable_tests(
    edittest
    logintest
    logouttest
    mediawikitest
    parsetest
    queryimageinfotest
    queryimagestest
    queryinfotest
    queryrevisiontest
    querysiteinfogeneraltest
    querysiteinfousergroupstest
    uploadtest
)

macro(MEDIAWIKI_TEST_RESOURCES)

    foreach(resource ${ARGN})

        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${resource}"
                           COMMAND ${CMAKE_COMMAND} -E copy
                           "${CMAKE_CURRENT_SOURCE_DIR}/${resource}"
                           "${CMAKE_CURRENT_BINARY_DIR}/${resource}"
                           DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${resource}")
        list(APPEND test_resources_dest "${resource}" )

    endforeach()

endmacro()

MEDIAWIKI_TEST_RESOURCES(
    parsetest.rc
    queryrevisiontest.rc
    queryrevisiontest_content.rc
    queryrevisiontest_cuted.rc
    queryrevisiontest_onetitle.rc
    queryrevisiontest_parsetree.rc
    queryrevisiontest_rollback.rc
    queryrevisiontest_timestamponly.rc
    queryrevisiontest_useronly.rc
    uploadtest_image.jpg
)

add_custom_target(TestResources ALL DEPENDS ${test_resources_dest})
