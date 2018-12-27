#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(KF5AkonadiContact_FOUND)
    add_subdirectory(addressbook)
endif()

if(KF5FileMetaData_FOUND)
    add_subdirectory(filesindexer)
endif()
