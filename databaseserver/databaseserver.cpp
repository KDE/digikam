/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2012 by Gilles Caulier<caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "databaseserver.moc"

// Qt includes

#include <QtGlobal>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QProcess>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "pollthread.h"
#include "databaseserveradaptor.h"
#include "databaseparameters.h"
#include "databaseservererror.h"

namespace Digikam
{

class DatabaseServer::DatabaseServerPriv
{
public:

    DatabaseServerPriv()
    {
        databaseProcess = 0;
        app             = 0;
        pollThread      = 0;
        internalDBName  = QString("digikam");
    }

    QProcess*         databaseProcess;
    QString           internalDBName;
    QCoreApplication* app;
    PollThread*       pollThread;
};

DatabaseServer::DatabaseServer(QCoreApplication* const application)
    : QObject(application), d(new DatabaseServerPriv)
{
    d->app = application;

    if (qDBusRegisterMetaType<DatabaseServerError>() < 0)
    {
        kError()<<"Error while registering DatabaseServerError class.";
    }
}

DatabaseServer::~DatabaseServer()
{
    delete d;
}

void DatabaseServer::registerOnDBus()
{
    new DatabaseServerAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/DatabaseServer", this);
    QDBusConnection::sessionBus().registerService("org.kde.digikam.DatabaseServer");
}

void DatabaseServer::startPolling()
{
    d->pollThread = new PollThread(d->app);
    d->pollThread->start();

    connect(d->pollThread, SIGNAL(done()),
            this, SLOT(stopDatabaseProcess()));
}

/*
 * Starts the database management server.
 * TODO: Ensure that no other digikam dbms is running. Reusing this instance instead start a new one.
 * Maybe this can be done by DBUS communication or an PID file.
 */
bool DatabaseServer::startDatabaseProcess(QDBusVariant& error)
{
    return startDatabaseProcess(DatabaseParameters::MySQLDatabaseType(), error);
}

/*
 * Starts the database management server.
 * TODO: Ensure that no other digikam dbms is running. Reusing this instance instead start a new one.
 * Maybe this can be done by DBUS communication or an PID file.
 */
bool DatabaseServer::startDatabaseProcess(const QString& dbType, QDBusVariant& error)
{
    if (dbType == DatabaseParameters::MySQLDatabaseType())
    {
        //        return QVariant::fromValue(startMYSQLDatabaseProcess());
        error = QDBusVariant(QVariant::fromValue(startMYSQLDatabaseProcess()));
        return false;
    }
    else
    {
        kDebug() << "DBType ["<< dbType <<"] is not supported.";
        DatabaseServerError errorDetails(DatabaseServerError::NotSupported, QString("DBType [%0] is not supported.").arg(dbType));
        error = QDBusVariant(QVariant::fromValue(errorDetails));
        return false;
    }
}

/*
 * Starts the database management server.
 * TODO: Ensure that no other digikam dbms is running. Reusing this instance instead start a new one.
 * Maybe this can be done by DBUS communication or an PID file.
 */
DatabaseServerError DatabaseServer::startMYSQLDatabaseProcess()
{
    DatabaseServerError result;
    const QString dbType(DatabaseParameters::MySQLDatabaseType());
    DatabaseParameters internalServerParameters = DatabaseParameters::defaultParameters(dbType);

    //TODO Don't know if this is needed, because after the thread is finished, the database server manager should close
    d->pollThread->stop = false;

    // QString filepath = KStandardDirs::locate("data", "digikam/database/dbconfig.xml");

    //TODO Move the database command outside of the code to the dbconfig.xml file
    const QString mysqldPath(DatabaseConfigElement::element(dbType).dbServerCmd);
    //const QString mysqldPath("/usr/sbin/mysqld");

    if ( mysqldPath.isEmpty() || (mysqldPath.compare( QLatin1String( "SERVERCMD_MYSQL-NOTFOUND" )) == 0))
    {
        kDebug() << "No path to mysqld set in server configuration!";
        return DatabaseServerError(DatabaseServerError::StartError, i18n("No path to mysqld set in server configuration."));
    }

    // create the database directories if they don't exists
    //  const QString dataDir     = XdgBaseDirs::saveDir( "data", QLatin1String( "Digikam/db_data" ) );
    //  const QString akDir       = XdgBaseDirs::saveDir( "data", QLatin1String( "Digikam/" ) );
    //  const QString miscDir     = XdgBaseDirs::saveDir( "data", QLatin1String( "Digikam/db_misc" ) );
    //  const QString fileDataDir = XdgBaseDirs::saveDir( "data", QLatin1String( "Digikam/file_db_data" ) );

    KStandardDirs dirs;
    const QString akDir       = KStandardDirs::locateLocal("data", "digikam/");
    const QString dataDir     = KStandardDirs::locateLocal("data", "digikam/db_data");
    const QString miscDir     = KStandardDirs::locateLocal("data", "digikam/db_misc");
    const QString fileDataDir = KStandardDirs::locateLocal("data", "digikam/file_db_data");

    /*
    * TODO Move the database command outside of the code to the dbconfig.xml file.
    * Offer a variable to the dataDir. E.g. the command definition in the config file has to be: /usr/bin/mysql_install_db --user=digikam --datadir=$dataDir$
    */
    const QString mysqlInitCmd(QString::fromLatin1("%1 --user=digikam --datadir=%2").arg(DatabaseConfigElement::element(dbType).dbInitCmd, dataDir));

    if (!dirs.exists(akDir))
    {
        dirs.makeDir(akDir);
    }

    if (!dirs.exists(dataDir))
    {
        dirs.makeDir(dataDir);
    }

    if (!dirs.exists(miscDir))
    {
        dirs.makeDir(miscDir);
    }

    if (!dirs.exists(miscDir))
    {
        dirs.makeDir(miscDir);
    }

    const QString globalConfig = KStandardDirs::locate("data", "digikam/database/mysql-global.conf");
    const QString localConfig  = KStandardDirs::locate("data", "digikam/database/mysql-local.conf");
    const QString actualConfig = KStandardDirs::locateLocal( "data", QLatin1String( "digikam" ) ) + QLatin1String("/mysql.conf");

    if ( globalConfig.isEmpty() )
    {
        kDebug() << "Did not find MySQL server default configuration (mysql-global.conf)";
        return DatabaseServerError(DatabaseServerError::StartError, i18n("Did not find MySQL server default configuration (mysql-global.conf)."));
    }

    bool confUpdate = false;
    QFile actualFile ( actualConfig );

    // update conf only if either global (or local) is newer than actual
    if ( (QFileInfo( globalConfig ).lastModified() > QFileInfo( actualFile ).lastModified()) ||
         (QFileInfo( localConfig ).lastModified()  > QFileInfo( actualFile ).lastModified()) )
    {
        QFile globalFile( globalConfig );
        QFile localFile ( localConfig );

        if ( globalFile.open( QFile::ReadOnly ) && actualFile.open( QFile::WriteOnly ) )
        {
            actualFile.write( globalFile.readAll() );

            if ( !localConfig.isEmpty() )
            {
                if ( localFile.open( QFile::ReadOnly ) )
                {
                    actualFile.write( localFile.readAll() );
                    localFile.close();
                }
            }

            globalFile.close();
            actualFile.close();
            confUpdate = true;
        }
        else
        {
            QString  str = i18n("Unable to create MySQL server configuration file.\n"
                                "This means that either the default configuration file (mysql-global.conf) was not readable\n"
                                "or the target file (mysql.conf) could not be written.");
            kDebug() << str;
            return DatabaseServerError(DatabaseServerError::StartError, str);
        }
    }

    // MySQL doesn't like world writeable config files (which makes sense), but
    // our config file somehow ends up being world-writable on some systems for no
    // apparent reason nevertheless, so fix that
    const QFile::Permissions allowedPerms = actualFile.permissions() &
                                            (QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther);

    if ( allowedPerms != actualFile.permissions() )
    {
        actualFile.setPermissions( allowedPerms );
    }

    if ( dataDir.isEmpty() )
    {
        QString  str = i18n("Digikam server was not able to create database data directory");
        kDebug() << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    if ( akDir.isEmpty() )
    {
        QString  str = i18n("Digikam server was not able to create database log directory");
        kDebug() << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    if ( miscDir.isEmpty() )
    {
        QString  str = i18n("Digikam server was not able to create database misc directory");
        kDebug() << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    // move mysql error log file out of the way
    const QFileInfo errorLog( dataDir + QDir::separator() + QString::fromLatin1( "mysql.err" ) );

    if ( errorLog.exists() )
    {
        QFile logFile( errorLog.absoluteFilePath() );
        QFile oldLogFile( dataDir + QDir::separator() + QString::fromLatin1( "mysql.err.old" ) );

        if ( logFile.open( QFile::ReadOnly ) && oldLogFile.open( QFile::Append ) )
        {
            oldLogFile.write( logFile.readAll() );
            oldLogFile.close();
            logFile.close();
            logFile.remove();
        }
        else
        {
            kDebug() << "Failed to open MySQL error log.";
        }
    }

    // clear mysql ib_logfile's in case innodb_log_file_size option changed in last confUpdate
    if ( confUpdate )
    {
        QFile(dataDir + QDir::separator() + QString::fromLatin1( "ib_logfile0" )).remove();
        QFile(dataDir + QDir::separator() + QString::fromLatin1( "ib_logfile1" )).remove();
    }

    // synthesize the mysqld command
    QStringList arguments;
    arguments << QString::fromLatin1( "--defaults-file=%1/mysql.conf" ).arg( akDir );
    arguments << QString::fromLatin1( "--datadir=%1/" ).arg( dataDir );
    arguments << QString::fromLatin1( "--socket=%1/mysql.socket" ).arg( miscDir );

    // init db
    if (!QFile(dataDir + QDir::separator() + QString("mysql")).exists())
    {
        QProcess initProcess;
        initProcess.start( mysqlInitCmd );

        if ( !initProcess.waitForFinished())
        {
            QString  str = i18n("Could not start database init command.\n"
                                "Executable: %1\n"
                                "Process error:%2", mysqlInitCmd, initProcess.errorString());
            kDebug() << str;
            return DatabaseServerError(DatabaseServerError::StartError, str);
        }
    }

    d->databaseProcess = new QProcess();
    d->databaseProcess->start( mysqldPath, arguments );

    if ( !d->databaseProcess->waitForStarted() )
    {
        QString argumentStr =  arguments.join(", ");
        QString  str        =  i18n("Could not start database server.");
        str                 += i18n("<p>Executable: %1</p>", mysqldPath);
        str                 += i18n("<p>Arguments: %1</p>", argumentStr);
        str                 += i18n("<p>Process error: %1</p>", d->databaseProcess->errorString());
        kDebug() << str;

        delete d->databaseProcess;
        d->databaseProcess = 0;

        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    const QLatin1String initCon( "initConnection" );

    {
        QSqlDatabase db = QSqlDatabase::addDatabase( DatabaseParameters::MySQLDatabaseType(), initCon );
        db.setConnectOptions(QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir));
        db.setUserName(QString("root"));
        db.setDatabaseName( QString() ); // might not exist yet, then connecting to the actual db will fail

        if ( !db.isValid() )
        {
            kDebug() << "Invalid database object during database server startup";
        }

        bool opened = false;

        for ( int i = 0; i < 120; ++i )
        {
            opened = db.open();

            if ( opened )
            {
                break;
            }

            if ( d->databaseProcess->waitForFinished( 500 ) )
            {
                kDebug() << "Database process exited unexpectedly during initial connection!";
                kDebug() << "executable: " << mysqldPath;
                kDebug() << "arguments: " << arguments;
                kDebug() << "stdout: " << d->databaseProcess->readAllStandardOutput();
                kDebug() << "stderr: " << d->databaseProcess->readAllStandardError();
                kDebug() << "exit code: " << d->databaseProcess->exitCode();
                kDebug() << "process error: " << d->databaseProcess->errorString();

                QString  str = i18n("Database process exited unexpectedly during initial connection."
                                    "<p>Executable: %1</p>”"
                                    "<p>Process error: %2</p>",
                                    mysqldPath, d->databaseProcess->errorString());

                return DatabaseServerError(DatabaseServerError::StartError, str);
            }
        }

        if ( opened )
        {
            {
                QSqlQuery query( db );

                if ( !query.exec( QString::fromLatin1( "USE %1" ).arg( d->internalDBName ) ) )
                {
                    kDebug() << "Failed to use database" << d->internalDBName;
                    kDebug() << "Query error:" << query.lastError().text();
                    kDebug() << "Database error:" << db.lastError().text();
                    kDebug() << "Trying to create database now...";

                    if ( !query.exec( QLatin1String( "CREATE DATABASE digikam" ) ) )
                    {
                        QString  str = i18n("Failed to create database"
                                            "<p>Query error: %1</p>"
                                            "<p>Database error: %2</p>", query.lastError().text(), db.lastError().text());
                        kDebug() << str;
                        return DatabaseServerError(DatabaseServerError::StartError, str);
                    }
                    else
                    {
                        kDebug() << "Database was successfully created";
                    }
                }
            } // make sure query is destroyed before we close the db

            db.close();
        }
    }

    QSqlDatabase::removeDatabase( initCon );
    return result;
}

/*
 * Creates the initial database for the internal server instance.
 */
DatabaseServerError DatabaseServer::createDatabase()
{
    const QLatin1String initCon( "initConnection" );
    QSqlDatabase db = QSqlDatabase::addDatabase( "MYSQL", initCon );

    // Might not exist yet, then connecting to the actual db will fail.
    db.setDatabaseName( QString() );

    if ( !db.isValid() )
    {
        kDebug() << "Invalid database object during initial database connection";
    }

    if ( db.open() )
    {
        QSqlQuery query( db );

        if ( !query.exec( QString::fromLatin1( "USE %1" ).arg( d->internalDBName ) ) )
        {
            kDebug() << "Failed to use database" << d->internalDBName;
            kDebug() << "Query error:" << query.lastError().text();
            kDebug() << "Database error:" << db.lastError().text();
            kDebug() << "Trying to create database now...";

            if ( !query.exec( QLatin1String( "CREATE DATABASE digikam" ) ) )
            {
                QString str = i18n("Failed to create database"
                                   "<p>Query error: %1</p>"
                                   "<p>Database error: %2</p>",query.lastError().text(), db.lastError().text());
                kDebug() << str;
                return DatabaseServerError(DatabaseServerError::StartError, str);
            }
        }

        // make sure query is destroyed before we close the db
        db.close();
    }

    QSqlDatabase::removeDatabase( initCon );
    return DatabaseServerError(DatabaseServerError::NoErrors, QString());
}

/*
 * Terminates the databaser server process.
 * TODO: Ensure that no other digikam application is using this dbms. Keep the server alive.
 * Maybe this can be done by DBUS communication or an PID file.
 */
void DatabaseServer::stopDatabaseProcess()
{
    if ( !d->databaseProcess )
    {
        return;
    }

    d->databaseProcess->terminate();
    d->databaseProcess->waitForFinished();
    d->databaseProcess->~QProcess();
    d->databaseProcess = 0;
    d->pollThread->stop = true;
    d->pollThread->wait();
    d->app->exit(0);
}

/*
 * Returns true if the server process is running.
 */
bool DatabaseServer::isRunning()
{
    if (d->databaseProcess == 0)
    {
        return false;
    }

    return (d->databaseProcess->state() == QProcess::Running);
}

} // namespace Digikam
