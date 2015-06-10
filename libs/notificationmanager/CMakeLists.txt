#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015 by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libnotificationmanager_SRCS
    dnotificationwrapper.cpp
)

if(APPLE)
    set(libnotificationmanager_SRCS
        ${libnotificationmanager_SRCS}
        macnotification.mm
    )
endif()

add_library(notificationmanager_src OBJECT ${libnotificationmanager_SRCS})

