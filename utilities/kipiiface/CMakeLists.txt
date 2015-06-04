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

add_library(kipiiface STATIC ${libkipiiface_SRCS})

target_include_directories(kipiiface PRIVATE "$<TARGET_PROPERTY:digikammodels,INTERFACE_INCLUDE_DIRECTORIES>"
                                            "$<TARGET_PROPERTY:digikamalbum,INTERFACE_INCLUDE_DIRECTORIES>"
                                            "$<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>"
                                            "$<TARGET_PROPERTY:progressmanager,INTERFACE_INCLUDE_DIRECTORIES>"
                                            "$<TARGET_PROPERTY:fileactionmanager,INTERFACE_INCLUDE_DIRECTORIES>"

)
# TODO we shouldn't depend on digikamgui, but on a specific library providing what we need
target_link_libraries(kipiiface

                      digikamgui      # for digikamapp

                      digikamsettings # for applicationsettings

                      KF5::Kipi

)

