#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(akonadi)

if(KF5FileMetaData_FOUND)
    add_subdirectory(kfilemetadata)
endif()

if(KF5Sane_FOUND)
    add_subdirectory(ksane)
endif()
