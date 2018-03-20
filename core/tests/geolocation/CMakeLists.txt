#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(Marble_FOUND)
    add_subdirectory(editor)
    add_subdirectory(geoiface)
    add_subdirectory(kmlexport)
endif()
