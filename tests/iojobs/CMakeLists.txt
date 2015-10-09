#
# Copyright (c) 2015 by Mohamed Anwer, <m dot anwer at gmx dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(IOJobsTest_SRCS iojobstest.cpp)

add_executable(IOJobsTest ${IOJobsTest_SRCS})

ecm_mark_nongui_executable(IOJobsTest)

target_link_libraries(IOJobsTest
                      Qt5::Core
                      Qt5::Test
                      digikamgui
                     )
