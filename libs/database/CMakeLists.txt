#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

configure_file(dbconfigversion.h.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/dbconfigversion.h)

set(libdatabasecore_SRCS
    core/databasecorebackend.cpp
    core/databaseparameters.cpp
    core/databaseerrorhandler.cpp
    core/databaseguierrorhandler.cpp # FIXME used to be part of app, ok here?
    core/thumbnaildatabaseaccess.cpp
    core/thumbnaildb.cpp
    core/thumbnailschemaupdater.cpp
    core/databaseconfigelement.cpp
    core/sqlquery.cpp
    core/dbactiontype.cpp
    core/tagregion.cpp

    # FIXME moved from app, check
    core/dbstatdlg.cpp
    core/scancontroller.cpp
    core/dio.cpp
    core/syncjob.cpp
    core/imageattributeswatch.cpp

)

set(libdatabase_SRCS
    album/albumdb.cpp
    collection/collectionscanner.cpp
    collection/collectionmanager.cpp
    collection/collectionlocation.cpp
    collection/collectionscannerhints.cpp
    framework/databaseaccess.cpp
    framework/databasewatch.cpp
    framework/databasewatchadaptor.cpp
    framework/databasebackend.cpp
    framework/databaseoperationgroup.cpp
    framework/databasetransaction.cpp
    framework/databaseurl.cpp
    framework/databasechangesets.cpp
    framework/databaseconstants.cpp
    framework/databasethumbnailinfoprovider.cpp
    framework/databasechecker.cpp
    framework/databasecopymanager.cpp
    framework/databaseserverstarter.cpp
    framework/databaseservererror.cpp
    item/imagecomments.cpp
    item/imageextendedproperties.cpp
    item/imageinfo.cpp
    item/imageinfolist.cpp
    item/imageinfocache.cpp
    item/imagelister.cpp
    item/imagelisterreceiver.cpp
    item/imageposition.cpp
    item/imagecopyright.cpp
    item/imagequerybuilder.cpp
    item/imagescanner.cpp
    item/imagetagpair.cpp
    imagehistory/imagehistorygraph.cpp
    imagehistory/imagehistorygraphmodel.cpp
    faces/databaseface.cpp
    faces/facetags.cpp
    faces/facetagseditor.cpp
    utils/downloadhistory.cpp
    utils/namefilter.cpp
    utils/searchxml.cpp
    utils/schemaupdater.cpp
    utils/tagproperties.cpp
    utils/tagscache.cpp
)

# enable exceptions
kde_source_files_enable_exceptions(imagehistory/imagehistorygraph.cpp imagehistory/imagehistorygraphmodel.cpp)

set(libhaar_SRCS
    haar/haar.cpp
    haar/haariface.cpp
)

set(libimgqsort_SRCS
    imgqsort/imgqsort.cpp
)

# Shared libdigikamdatabase ########################################################

set(digikamdatabase_LIB_SRCS
        ${libdatabase_SRCS}
        ${libdatabasecore_SRCS}
        ${libdatabasemodels_SRCS}
        ${libhaar_SRCS}
        ${pgfutils_SRCS}
        ${libimgqsort_SRCS}
)

add_library(digikamdatabase_src OBJECT ${digikamdatabase_LIB_SRCS})
add_library(digikamdatabase SHARED $<TARGET_OBJECTS:digikamdatabase_src>)


# TODO clean this up by changing the includes using database to use longer path?
target_include_directories(digikamdatabase
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/album>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/collection>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/framework>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/item>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/faces>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/haar>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgqsort>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imagehistory>
)

target_link_libraries(digikamdatabase
                      PRIVATE
                      dmetadata
                      digikamwidgets
                      dbjobs
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql

                      KF5::KIOCore

                      KF5::KExiv2
                      KF5::KDcraw

                      ${PGF_LIBRARIES}
)

if(KF5KFace_FOUND)
    target_link_libraries(digikamdatabase PRIVATE ${KF5KFace_LIBRARIES})
endif()

if(KF5KGeoMap_FOUND)
    target_link_libraries(digikamdatabase PRIVATE ${KF5KGeoMap_LIBRARIES})
endif()

if(Baloo_FOUND) # circular linking, because we need baloowrap.h path.
    target_link_libraries(digikamdatabase PRIVATE baloowrap)
endif()

# for imgqsort, does it even belong to database?
if(OpenCV_FOUND)
    target_link_libraries(digikamdatabase PRIVATE ${OpenCV_LIBRARIES})
endif()

# digikamdatabase static for linking with other static libs

add_library(digikamdatabase_static STATIC $<TARGET_OBJECTS:digikamdatabase_src>)


# TODO clean this up by changing the includes using database to use longer path?
target_include_directories(digikamdatabase_static
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/album>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/collection>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/framework>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/item>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/faces>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/haar>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgqsort>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imagehistory>
)

target_link_libraries(digikamdatabase_static
                      PRIVATE
                      dmetadata
                      digikamwidgets
)

set_target_properties(digikamdatabase PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

install(TARGETS digikamdatabase ${INSTALL_TARGETS_DEFAULT_ARGS} )
