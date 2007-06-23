ADD_SUBDIRECTORY( sqlite2 )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database/sqlite2
		     ${SQLITE_INCLUDE_DIR}		     
                   )

SET(libdatabase_STAT_SRCS albumdb.cpp 
                          albumdb_sqlite2.cpp 
                          collectionscanner.cpp 
                          collectionmanager.cpp 
                          collectionlocation.cpp 
                          databaseaccess.cpp 
                          databaseattributeswatch.cpp 
                          databasebackend.cpp 
                          databaseparameters.cpp 
                          databasetransaction.cpp
                          databaseurl.cpp 
                          imageinfo.cpp 
                          imageinfolist.cpp 
                          imageinfocache.cpp 
                          imagelister.cpp 
                          imagelisterreceiver.cpp 
                          imagequerybuilder.cpp 
                          namefilter.cpp 
                          schemaupdater.cpp 
                          backendsqlite3.cpp 
                          upgradedb_sqlite2tosqlite3.cpp 
   )

KDE4_AUTOMOC(${libdatabase_STAT_SRCS})

KDE4_ADD_LIBRARY(libdatabase STATIC ${libdatabase_STAT_SRCS})

TARGET_LINK_LIBRARIES(libdatabase libsqlite2 
                                  ${KDE4_KIO_LIBS} 
                                  ${KDCRAW_LIBRARIES} 
                                  ${KEXIV2_LIBRARIES} 
                                  ${SQLITE_LIBRARIES}
                     )

INSTALL( FILES albumdb.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )


