#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
# Copyright (c) 2015      by Mohamed_Anwer, <m_dot_anwer at gmx dot com>
# Copyright (C) 2018      by Mario Frank,   <mario dot frank at uni minus potsdam dot de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

# Boost uses operator names (and, not, ...)
string(REPLACE "-fno-operator-names" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(libdatabasemodels_SRCS
    models/itemfiltermodel.cpp
    models/itemfiltermodel_p.cpp
    models/itemfiltermodelthreads.cpp
    models/itemfiltersettings.cpp
    models/itemversionsmodel.cpp
    models/itemthumbnailmodel.cpp
    models/itemsortsettings.cpp
    models/itemlistmodel.cpp
    models/itemmodel.cpp
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
    thumbsdb/thumbsdbschemaupdater.cpp
    thumbsdb/thumbsdbbackend.cpp
    thumbsdb/thumbsdbaccess.cpp
)

set(libdatabaseutils_SRCS
    utils/widgets/dbstatdlg.cpp
    utils/widgets/dbsettingswidget.cpp
    utils/widgets/dbmigrationdlg.cpp
    utils/ifaces/dbinfoiface.cpp
    utils/ifaces/syncjob.cpp
    utils/ifaces/dio.cpp
    utils/scan/scancontroller.cpp
    utils/scan/scancontroller_p.cpp
)

set(libdatabase_SRCS
    collection/collectionscanner.cpp
    collection/collectionscanner_p.cpp
    collection/collectionscanner_scan.cpp
    collection/collectionscanner_utils.cpp
    collection/collectionmanager.cpp
    collection/collectionmanager_p.cpp
    collection/collectionmanager_location.cpp
    collection/collectionmanager_album.cpp
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

    item/containers/iteminfo.cpp
    item/containers/iteminfolist.cpp
    item/containers/iteminfocache.cpp
    item/containers/itemcomments.cpp
    item/containers/itemcopyright.cpp
    item/containers/itemposition.cpp
    item/containers/itemtagpair.cpp
    item/containers/itemextendedproperties.cpp

    item/lister/itemlister.cpp
    item/lister/itemlister_palbum.cpp
    item/lister/itemlister_talbum.cpp
    item/lister/itemlister_salbum.cpp
    item/lister/itemlister_falbum.cpp
    item/lister/itemlisterrecord.cpp
    item/lister/itemlisterreceiver.cpp
    item/lister/itemattributeswatch.cpp

    item/query/itemquerybuilder.cpp
    item/query/itemquerybuilder_p.cpp
    item/query/itemqueryposthooks.cpp
    item/query/fieldquerybuilder.cpp

    item/scanner/itemscanner.cpp
    item/scanner/itemscanner_p.cpp
    item/scanner/itemscanner_database.cpp
    item/scanner/itemscanner_file.cpp
    item/scanner/itemscanner_photo.cpp
    item/scanner/itemscanner_video.cpp
    item/scanner/itemscanner_history.cpp
    item/scanner/itemscanner_baloo.cpp

    history/itemhistorygraph.cpp
    history/itemhistorygraphmodel.cpp

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
        item/containers/itemgps.cpp
       )
endif()

# enable exceptions
kde_source_files_enable_exceptions(imagehistory/imagehistorygraph.cpp imagehistory/imagehistorygraphmodel.cpp)

set(libhaar_SRCS
    haar/haar.cpp
    haar/haariface.cpp
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
add_library(digikamdatabasemain_src OBJECT ${libdatabaseutils_SRCS})
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
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/item/containers>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/item/lister>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/item/query>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/item/scanner>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tags>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/haar>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imagehistory>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/server>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thumbsdb>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils/widgets>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils/ifaces>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils/scan>
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
