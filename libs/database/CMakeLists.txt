#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
# Copyright (c) 2015      by Mohamed Anwer, <m dot anwer at gmx dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libdatabasecore_SRCS
    core/databasecorebackend.cpp
    core/databaseparameters.cpp
    core/databaseerrorhandler.cpp
    core/databaseconfigelement.cpp
    core/sqlquery.cpp
    core/dbactiontype.cpp
    core/tagregion.cpp

    thumbnails/thumbnaildatabaseaccess.cpp
    thumbnails/thumbnaildb.cpp
    thumbnails/thumbnailschemaupdater.cpp
)

#this sources must be set in digikam main app
set(libdatabasemain_SRCS
    core/databaseguierrorhandler.cpp
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
    dbjobs/dbjobsthread.cpp
    dbjobs/dbjob.cpp
    dbjobs/dbjobinfo.cpp
    dbjobs/dbjobsmanager.cpp
    dbjobs/duplicatesprogressobserver.cpp
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
    facesiface/databaseface.cpp
    facesiface/facetags.cpp
    facesiface/facetagseditor.cpp
    utils/downloadhistory.cpp
    utils/namefilter.cpp
    utils/searchxml.cpp
    utils/schemaupdater.cpp
    utils/tagproperties.cpp
    utils/tagscache.cpp
)

if(MARBLE_FOUND)
    set(libdatabase_SRCS
        ${libdatabase_SRCS}
        item/imagegps.cpp
       )
endif()

# enable exceptions
kde_source_files_enable_exceptions(imagehistory/imagehistorygraph.cpp imagehistory/imagehistorygraphmodel.cpp)

set(libhaar_SRCS
    haar/haar.cpp
    haar/haariface.cpp
)

# Also part of digikam main app
set(libimgqsort_SRCS
    imgqsort/imgqsort.cpp
)

# Shared libdigikamdatabase ########################################################

set(digikamdatabase_LIB_SRCS
        ${libdatabase_SRCS}
        ${libhaar_SRCS}
        ${pgfutils_SRCS}
)

include_directories(
    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Completion,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(digikamdatabasemain_src OBJECT ${libdatabasemain_SRCS} ${libimgqsort_SRCS})
add_library(digikamdatabase_src OBJECT ${digikamdatabase_LIB_SRCS} )
add_library(digikamdatabasecore_src OBJECT  ${libdatabasecore_SRCS} )
add_library(digikamdatabase SHARED $<TARGET_OBJECTS:digikamdatabase_src> $<TARGET_OBJECTS:digikamdatabasemodels_src>)

generate_export_header(digikamdatabase BASE_NAME digikam_database EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/../../app/utils/digikam_database_export.h")

target_include_directories(digikamdatabase
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/album>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/collection>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/framework>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dbjobs>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/item>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/facesiface>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/haar>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgqsort>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imagehistory>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thumbnails>
)

target_link_libraries(digikamdatabase

                      PRIVATE
                      digikamcore

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql
                      Qt5::DBus

                      KF5::KIOCore
                      KF5::Solid
                      KF5::I18n

                      ${PGF_LIBRARIES}
)

if(KF5FileMetaData_FOUND)
    target_link_libraries(digikamdatabase PRIVATE baloowrap)
endif()

set_target_properties(digikamdatabase PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

install(TARGETS digikamdatabase ${INSTALL_TARGETS_DEFAULT_ARGS} )
