#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libkipiiface_SRCS
    kipiimagecollection.cpp
    kipiimagecollectionselector.cpp
    kipiuploadwidget.cpp
    kipiinterface.cpp
    kipiimageinfo.cpp
    kipipluginloader.cpp
)

add_library(kipiiface_src OBJECT ${libkipiiface_SRCS})

#add_library(kipiiface STATIC $<TARGET_OBJECTS:kipiiface_src> $<TARGET_OBJECTS:digikamgui_src>)

#target_link_libraries(kipiiface
#                      PRIVATE
#                      digikamdatabase

#                      digikamsettings # for applicationsettings

#                      KF5::Kipi

#)

