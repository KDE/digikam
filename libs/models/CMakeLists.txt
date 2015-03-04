#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libdatabasemodels_SRCS
    imagemodel.cpp
    imagefiltermodel.cpp
    imagefiltermodelpriv.cpp
    imagefiltermodelthreads.cpp
    imagefiltersettings.cpp
    imagelistmodel.cpp
    imagesortsettings.cpp
    imagethumbnailmodel.cpp
    imageversionsmodel.cpp
)

set(libalbummodels_SRCS
    imagealbummodel.cpp
    imagealbumfiltermodel.cpp
    abstractalbummodel.cpp
    albummodel.cpp
    albumfiltermodel.cpp
    albummodeldragdrophandler.cpp
)

set(libgenericmodels_SRCS
    categorizeditemmodel.cpp
)

add_library(digikammodels STATIC ${libdatabasemodels_SRCS} ${libalbummodels_SRCS} ${libgenericmodels_SRCS})

target_link_libraries(digikammodels

                      filters

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::DBus
)
