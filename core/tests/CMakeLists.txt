#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# enabling for everything although only required by DImg.
kde_enable_exceptions()

add_subdirectory(albummodel)
add_subdirectory(advancedrename)
add_subdirectory(assistants)
add_subdirectory(database)
add_subdirectory(dialogs)
add_subdirectory(dimg)
add_subdirectory(dmetadata)
add_subdirectory(facesengine)
add_subdirectory(geolocation)
add_subdirectory(imgqsort)
add_subdirectory(iojobs)
add_subdirectory(multithreading)
add_subdirectory(fileio)
add_subdirectory(filters)
add_subdirectory(timestampupdate)
add_subdirectory(widgets)
add_subdirectory(rawengine)
add_subdirectory(presentation)
add_subdirectory(mediaserver)
add_subdirectory(webservices)


if(ENABLE_MEDIAPLAYER)
    add_subdirectory(video)
endif()

if(ENABLE_DIGIKAM_MODELTEST)
    message(STATUS "Modeltest enabled")
    set(libdigikamgui_SRCS ${libdigikamgui_SRCS}
        modeltest/modeltest.cpp
       )
    add_definitions(-DENABLE_DIGIKAM_MODELTEST)
endif()
