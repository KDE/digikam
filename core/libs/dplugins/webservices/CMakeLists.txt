#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# OAuth2 library ---------------------------------------------------

# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY)

# Remove unwanted warnings from O2 library 
REMOVE_COMPILER_WARNINGS("-pedantic")
REMOVE_COMPILER_WARNINGS("-Wextra-semi")

# It also does not export symbols, so export all by default
unset(CMAKE_CXX_VISIBILITY_PRESET)

# Copied from o2/src/CMakeLists.txt
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(libwso2_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2reply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2replyserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2requestor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2simplecrypt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o0settingsstore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o0baseauth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o0abstractstore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o0globals.h
    # Enable when needed
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1requestor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1timedreply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1smugmug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1twitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/oxtwitter.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1dropbox.h
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1flickr.h
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2gft.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2facebook.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2skydrive.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2hubic.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2uber.cpp
)

add_library(libwso2_src OBJECT ${libwso2_SRCS})
