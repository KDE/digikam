#
# Copyright (c) 2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(ksaneiface_SRCS kscaniface.cpp scandialog.cpp saveimgthread.cpp)

add_library(ksaneiface_src OBJECT ${ksaneiface_SRCS})
add_library(ksaneiface STATIC $<TARGET_OBJECTS:ksaneiface_src>)

target_link_libraries(ksaneiface PRIVATE ${LIBKSANE_LIBRARIES} KF5::I18n)

