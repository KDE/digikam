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

#for digikam core lib
add_library(digikamgenericmodels_src OBJECT ${libgenericmodels_SRCS})

add_library(dgikamdatabasemodels_src OBJECT ${libdatabasemodels_SRCS})
add_library(digikammodels_src OBJECT  ${libalbummodels_SRCS} ${libgenericmodels_SRCS})
add_library(digikammodels STATIC $<TARGET_OBJECTS:digikammodels_src>)

target_link_libraries(digikammodels

                      PRIVATE
                      filters
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      Qt5::Sql
                      Qt5::DBus
)
