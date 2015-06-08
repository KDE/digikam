#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(baloowrap_SRCS baloowrap.cpp)

add_library(baloowrap_src OBJECT ${baloowrap_SRCS})
add_library(baloowrap STATIC $<TARGET_OBJECTS:baloowrap_src>)

target_link_libraries(baloowrap  PRIVATE KF5::FileMetaData KF5::I18n)

#target_include_directories(baloowrap PRIVATE "$<TARGET_PROPERTY:KF5::FileMetadata,INTERFACE_INCLUDE_DIRECTORIES>")
