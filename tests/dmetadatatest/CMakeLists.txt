#
# Copyright (c) 2015 by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(testTag_SRCS tagsreadwritetest.cpp)
add_executable(tagsReadWriteTest ${testTag_SRCS})
ecm_mark_nongui_executable(tagsReadWriteTest)
target_link_libraries(tagsReadWriteTest
                        Qt5::Core
                        Qt5::Test
                        digikamcore
                        KF5::KExiv2
                     )

if(${KF5KGeoMap_FOUND})
    target_link_libraries(tagsReadWriteTest ${KF5KGeoMap_LIBRARIES})
endif() 
