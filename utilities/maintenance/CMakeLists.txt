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

if(${KF5KFace_FOUND})
    target_link_libraries(maintenance facemanagement)
endif()


target_include_directories(maintenance PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)


target_link_libraries(maintenance
                      digikamgui # for DigikamApp...
                      digikamdatabase
                      notificationmanager
                      progressmanager
)
