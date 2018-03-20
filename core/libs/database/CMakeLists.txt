#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
# Copyright (c) 2015      by Mohamed Anwer, <m dot anwer at gmx dot com>
# Copyright (C)      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

# Boost uses operator names (and, not, ...)
string(REPLACE "-fno-operator-names" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(libdatabasemodels_SRCS
    models/imagemodel.cpp
    models/imagefiltermodel.cpp
    models/imagefiltermodelpriv.cpp
    models/imagefiltermodelthreads.cpp
    models/imagefiltersettings.cpp
    models/imagelistmodel.cpp
    models/imagesortsettings.cpp
    models/imagethumbnailmodel.cpp
    models/imageversionsmodel.cpp
)

set(libdatabasecore_SRCS
    server/databaseserverstarter.cpp
    server/databaseservererror.cpp
    server/databaseserver.cpp

    engine/dbengineconfigloader.cpp
    engine/dbengineconfig.cpp
    engine/dbengineactiontype.cpp
    engine/dbengineerrorhandler.cpp
    engine/dbengineguierrorhandler.cpp
    engine/dbengineparameters.cpp
    engine/dbenginebackend.cpp
    engine/dbenginesqlquery.cpp
    engine/dbengineaccess.cpp

    tags/tagregion.cpp

    thumbsdb/thumbsdb.cpp
    thumbsdb/thumbsdbchemaupdater.cpp
    thumbsdb/thumbsdbbackend.cpp
    thumbsdb/thumbsdbaccess.cpp
)

set(libdatabaseutils_SRCS
    utils/dbstatdlg.cpp
    utils/dbsettingswidget.cpp
    utils/dbmigrationdlg.cpp
    utils/dbinfoiface.cpp
    utils/dio.cpp
    utils/scancontroller.cpp
    utils/syncjob.cpp
)

set(libdatabase_SRCS
    collection/collectionscanner.cpp
    collection/collectionmanager.cpp
    collection/collectionlocation.cpp
    collection/collectionscannerhints.cpp

    coredb/coredb.cpp
    coredb/coredbsearchxml.cpp
    coredb/coredbchecker.cpp
    coredb/coredbcopymanager.cpp
    coredb/coredbconstants.cpp
    coredb/coredbtransaction.cpp
    coredb/coredbschemaupdater.cpp
    coredb/coredbchangesets.cpp
    coredb/coredbthumbinfoprovider.cpp
    coredb/coredboperationgroup.cpp
    coredb/coredbbackend.cpp
    coredb/coredbwatch.cpp
    coredb/coredburl.cpp
    coredb/coredbaccess.cpp
    coredb/coredbnamefilter.cpp
    coredb/coredbdownloadhistory.cpp

    tags/tagproperties.cpp
    tags/tagscache.cpp
    tags/facetags.cpp
    tags/facetagseditor.cpp
    tags/facetagsiface.cpp

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
    item/imageattributeswatch.cpp

    imagehistory/imagehistorygraph.cpp
    imagehistory/imagehistorygraphmodel.cpp

    similaritydb/similaritydb.cpp
    similaritydb/similaritydbschemaupdater.cpp
    similaritydb/similaritydbbackend.cpp
    similaritydb/similaritydbaccess.cpp
)

if(ENABLE_DBUS)
    set(libdatabase_SRCS
        ${libdatabase_SRCS}
        coredb/coredbwatchadaptor.cpp
       )
endif()

if(Marble_FOUND)
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
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::Solid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::WidgetsAddons,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_library(digikamdatabase_src     OBJECT ${digikamdatabase_LIB_SRCS} ${libdatabasemodels_SRCS})
add_library(digikamdatabasemain_src OBJECT ${libdatabaseutils_SRCS} ${libimgqsort_SRCS})
add_library(digikamdatabasecore_src OBJECT ${libdatabasecore_SRCS})
add_library(digikamdatabase         $<TARGET_OBJECTS:digikamdatabase_src>)

generate_export_header(digikamdatabase
                       BASE_NAME digikam_database
                       EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/../../app/utils/digikam_database_export.h"
)

target_include_directories(digikamdatabase
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/coredb>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/collection>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dbjobs>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/item>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tags>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/haar>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgqsort>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imagehistory>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/server>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thumbsdb>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
)

target_link_libraries(digikamdatabase

                      PRIVATE
                      digikamcore

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql

                      KF5::Solid
                      KF5::I18n
)

if(ENABLE_DBUS)
    target_link_libraries(digikamdatabase PRIVATE Qt5::DBus)
endif()

if(KF5FileMetaData_FOUND)
    target_link_libraries(digikamdatabase PRIVATE baloowrap)
endif()

set_target_properties(digikamdatabase PROPERTIES VERSION ${DIGIKAM_VERSION_SHORT} SOVERSION ${DIGIKAM_VERSION_SHORT})

install(TARGETS digikamdatabase ${INSTALL_TARGETS_DEFAULT_ARGS} )
