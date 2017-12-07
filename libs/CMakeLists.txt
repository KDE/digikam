#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

#---------------------------------

# Common DNG SDK definitions for DNGWriter and RAWEngine.
#
# Set platteforms flags.
# NOTE: see bug #195735: do not enable Mac flags provided by Adobe.
#       Sounds like all compile fine like under Linux.

if(MSVC)
    add_definitions(
                    # XMP SDK
                    -DWIN_ENV=1

                    # DNG SDK
                    -DqWinOS=1
                    -DqMacOS=0
                    -DqDNGUseStdInt=0
                    )
else()
    add_definitions(
                     # XMP SDK
                    -DUNIX_ENV=1

                    # DNG SDK
                    -DqWinOS=0
                    -DqMacOS=0
                    -DqDNGUseStdInt=1
                   )
endif()

# Check processor endianness
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if(NOT IS_BIG_ENDIAN)
    add_definitions(-DqDNGLittleEndian=1)
endif()

# Thread safe support under Mac and Linux using pthread library

if(NOT MSVC)
    add_definitions(-DqDNGThreadSafe=1)
endif()

# Level of debug info in the console.

add_definitions(-DqDNGDebug=0)
add_definitions(-DqDNGValidateTarget=1)

#---------------------------------

add_subdirectory(dimg)
add_subdirectory(dmetadata)
add_subdirectory(database)
add_subdirectory(dngwriter)
add_subdirectory(dtrash)
add_subdirectory(facesengine)
add_subdirectory(iojobs)
add_subdirectory(jpegutils)
add_subdirectory(pgfutils)
add_subdirectory(threadimageio)
add_subdirectory(widgets)
add_subdirectory(imageproperties)
add_subdirectory(progressmanager)
add_subdirectory(threads)
add_subdirectory(versionmanager)
add_subdirectory(notificationmanager)
add_subdirectory(models)
add_subdirectory(template)
add_subdirectory(dialogs)
add_subdirectory(kmemoryinfo)
add_subdirectory(fileactionmanager)
add_subdirectory(filters)
add_subdirectory(settings)
add_subdirectory(rawengine)
add_subdirectory(album)
add_subdirectory(tags)
add_subdirectory(transitionmngr)
