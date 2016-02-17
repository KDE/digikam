#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(firstrun)
add_subdirectory(common)
add_subdirectory(expoblending)

if(KF5CalendarCore_FOUND)
    add_subdirectory(calendar)
endif()
