#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(icons)
add_subdirectory(pics)
add_subdirectory(about)
add_subdirectory(colorschemes)
add_subdirectory(database)
add_subdirectory(facesengine)
add_subdirectory(geolocation)
add_subdirectory(metadata)
add_subdirectory(profiles)
add_subdirectory(hotplug)
add_subdirectory(scripts)
add_subdirectory(filters)
add_subdirectory(printcreator)

if(LibXml2_FOUND AND LibXslt_FOUND)
    add_subdirectory(htmlgallery)
endif()
