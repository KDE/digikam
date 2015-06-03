#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(fileactionmanager_SRCS
    metadatahub.cpp
    fileactionprogress.cpp
    fileactionmngr.cpp
    fileactionmngr_p.cpp
    imageinfotasksplitter.cpp
    databaseworkeriface.cpp
    fileworkeriface.cpp
    fileactionimageinfolist.cpp
)

add_library(fileactionmanager STATIC ${fileactionmanager_SRCS})

target_include_directories(fileactionmanager
                           PRIVATE
                           "$<TARGET_PROPERTY:baloowrap,INTERFACE_INCLUDE_DIRECTORIES>")
target_link_libraries(fileactionmanager
                      
                      progressmanager
                      dmetadata
)
