#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libmaintenance_SRCS
    imageinfojob.cpp
    imageinfoalbumsjob.cpp
    metadatatask.cpp
    metadatasynchronizer.cpp
    duplicatesfinder.cpp
    newitemsfinder.cpp
    thumbsgenerator.cpp
    thumbstask.cpp
    fingerprintsgenerator.cpp
    fingerprintstask.cpp
    imagequalitysorter.cpp
    imagequalitysettings.cpp
    imagequalitytask.cpp
    maintenancedlg.cpp
    maintenancemngr.cpp
    maintenancetool.cpp
    maintenancesettings.cpp
    maintenancethread.cpp
)

# TODO figure out how to do this without having separate target_link_libraries later..
if(${KF5KFace_FOUND})
    set(libmaintenance_SRCS
        ${libmaintenance_SRCS}
        facesdetector.cpp
       )
endif()

add_library(maintenance STATIC ${libmaintenance_SRCS})

target_link_libraries(maintenance
                      PRIVATE
                      digikamgui # for DigikamApp...


                      digikamdatabase
                      notificationmanager
                      progressmanager

                      KF5::KDcraw
                      KF5::KExiv2
                      KF5::Kipi # dependecy from digikamgui

)

if(${KF5KFace_FOUND})
    target_link_libraries(maintenance PUBLIC facemanagement)
endif()
