/*
 * databaseserver.cpp
 *
 *  Created on: 27.11.2009
 *      Author: mueller
 */

#include "databaseserver.h"

// Qt includes

#include <QString>
#include <QtGlobal>
#include <QFile>

void DatabaseServer::startDatabaseProcess()
{
  if ( !DbConfig::useInternalServer() )
    return;

  const QString mysqldPath = DbConfig::serverPath();
  if ( mysqldPath.isEmpty() )
    akFatal() << "No path to mysqld set in server configuration!";

  // create the database directories if they don't exists
  const QString dataDir = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi/db_data" ) );
  const QString akDir   = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi/" ) );
  const QString miscDir = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi/db_misc" ) );
  const QString fileDataDir = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi/file_db_data" ) );

  // generate config file
  const QString globalConfig = XdgBaseDirs::findResourceFile( "config", QLatin1String( "akonadi/mysql-global.conf" ) );
  const QString localConfig  = XdgBaseDirs::findResourceFile( "config", QLatin1String( "akonadi/mysql-local.conf" ) );
  const QString actualConfig = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi" ) ) + QLatin1String("/mysql.conf");
  if ( globalConfig.isEmpty() )
    akFatal() << "Did not find MySQL server default configuration (mysql-global.conf)";
  bool confUpdate = false;
  QFile actualFile ( actualConfig );
  // update conf only if either global (or local) is newer than actual
  if ( (QFileInfo( globalConfig ).lastModified() > QFileInfo( actualFile ).lastModified()) ||
       (QFileInfo( localConfig ).lastModified()  > QFileInfo( actualFile ).lastModified()) )
  {
    QFile globalFile( globalConfig );
    QFile localFile ( localConfig );
    if ( globalFile.open( QFile::ReadOnly ) && actualFile.open( QFile::WriteOnly ) ) {
      actualFile.write( globalFile.readAll() );
      if ( !localConfig.isEmpty() ) {
        if ( localFile.open( QFile::ReadOnly ) ) {
          actualFile.write( localFile.readAll() );
          localFile.close();
        }
      }
      globalFile.close();
      actualFile.close();
      confUpdate = true;
    } else {
      akError() << "Unable to create MySQL server configuration file.";
      akError() << "This means that either the default configuration file (mysql-global.conf) was not readable";
      akFatal() << "or the target file (mysql.conf) could not be written.";
    }
  }

  // MySQL doesn't like world writeable config files (which makes sense), but
  // our config file somehow ends up being world-writable on some systems for no
  // apparent reason nevertheless, so fix that
  const QFile::Permissions allowedPerms = actualFile.permissions()
      & ( QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther );
  if ( allowedPerms != actualFile.permissions() )
    actualFile.setPermissions( allowedPerms );

  if ( dataDir.isEmpty() )
    akFatal() << "Akonadi server was not able not create database data directory";

  if ( akDir.isEmpty() )
    akFatal() << "Akonadi server was not able not create database log directory";

  if ( miscDir.isEmpty() )
    akFatal() << "Akonadi server was not able not create database misc directory";

  // move mysql error log file out of the way
  const QFileInfo errorLog( dataDir + QDir::separator() + QString::fromLatin1( "mysql.err" ) );
  if ( errorLog.exists() ) {
    QFile logFile( errorLog.absoluteFilePath() );
    QFile oldLogFile( dataDir + QDir::separator() + QString::fromLatin1( "mysql.err.old" ) );
    if ( logFile.open( QFile::ReadOnly ) && oldLogFile.open( QFile::Append ) ) {
      oldLogFile.write( logFile.readAll() );
      oldLogFile.close();
      logFile.close();
      logFile.remove();
    } else {
      akError() << "Failed to open MySQL error log.";
    }
  }

  // clear mysql ib_logfile's in case innodb_log_file_size option changed in last confUpdate
  if ( confUpdate ) {
      QFile(dataDir + QDir::separator() + QString::fromLatin1( "ib_logfile0" )).remove();
      QFile(dataDir + QDir::separator() + QString::fromLatin1( "ib_logfile1" )).remove();
  }

  // synthesize the mysqld command
  QStringList arguments;
  arguments << QString::fromLatin1( "--defaults-file=%1/mysql.conf" ).arg( akDir );
  arguments << QString::fromLatin1( "--datadir=%1/" ).arg( dataDir );
  arguments << QString::fromLatin1( "--socket=%1/mysql.socket" ).arg( miscDir );

  mDatabaseProcess = new QProcess( this );
  mDatabaseProcess->start( mysqldPath, arguments );
  if ( !mDatabaseProcess->waitForStarted() ) {
    akError() << "Could not start database server!";
    akError() << "executable:" << mysqldPath;
    akError() << "arguments:" << arguments;
    akFatal() << "process error:" << mDatabaseProcess->errorString();
  }

  const QLatin1String initCon( "initConnection" );
  {
    QSqlDatabase db = QSqlDatabase::addDatabase( DbConfig::driverName(), initCon );
    DbConfig::configure( db );
    db.setDatabaseName( QString() ); // might not exist yet, then connecting to the actual db will fail
    if ( !db.isValid() )
      akFatal() << "Invalid database object during database server startup";

    bool opened = false;
    for ( int i = 0; i < 120; ++i ) {
      opened = db.open();
      if ( opened )
        break;
      if ( mDatabaseProcess->waitForFinished( 500 ) ) {
        akError() << "Database process exited unexpectedly during initial connection!";
        akError() << "executable:" << mysqldPath;
        akError() << "arguments:" << arguments;
        akError() << "stdout:" << mDatabaseProcess->readAllStandardOutput();
        akError() << "stderr:" << mDatabaseProcess->readAllStandardError();
        akError() << "exit code:" << mDatabaseProcess->exitCode();
        akFatal() << "process error:" << mDatabaseProcess->errorString();
      }
    }

    if ( opened ) {
      {
        QSqlQuery query( db );
        if ( !query.exec( QString::fromLatin1( "USE %1" ).arg( DbConfig::databaseName() ) ) ) {
          akDebug() << "Failed to use database" << DbConfig::databaseName();
          akDebug() << "Query error:" << query.lastError().text();
          akDebug() << "Database error:" << db.lastError().text();
          akDebug() << "Trying to create database now...";
          if ( !query.exec( QLatin1String( "CREATE DATABASE akonadi" ) ) ) {
            akError() << "Failed to create database";
            akError() << "Query error:" << query.lastError().text();
            akFatal() << "Database error:" << db.lastError().text();
          }
        }
      } // make sure query is destroyed before we close the db
      db.close();
    }
  }

  QSqlDatabase::removeDatabase( initCon );
}

void DatabaseServer::createDatabase()
{
  const QLatin1String initCon( "initConnection" );
  QSqlDatabase db = QSqlDatabase::addDatabase( DbConfig::driverName(), initCon );
  DbConfig::configure( db );
  db.setDatabaseName( QString() ); // might not exist yet, then connecting to the actual db will fail
  if ( !db.isValid() )
    akFatal() << "Invalid database object during initial database connection";

  if ( db.open() ) {
    {
      QSqlQuery query( db );
      if ( !query.exec( QString::fromLatin1( "USE %1" ).arg( DbConfig::databaseName() ) ) ) {
        akDebug() << "Failed to use database" << DbConfig::databaseName();
        akDebug() << "Query error:" << query.lastError().text();
        akDebug() << "Database error:" << db.lastError().text();
        akDebug() << "Trying to create database now...";
        if ( !query.exec( QLatin1String( "CREATE DATABASE akonadi" ) ) ) {
          akError() << "Failed to create database";
          akError() << "Query error:" << query.lastError().text();
          akFatal() << "Database error:" << db.lastError().text();
        }
      }
    } // make sure query is destroyed before we close the db
    db.close();
  }
  QSqlDatabase::removeDatabase( initCon );
}

void DatabaseServer::stopDatabaseProcess()
{
  if ( !mDatabaseProcess )
    return;
  mDatabaseProcess->terminate();
  mDatabaseProcess->waitForFinished();
}
