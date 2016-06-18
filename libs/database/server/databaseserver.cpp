/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : Mysql internal database server
 *
 * Copyright (C) 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016 by Swati Lodha <swatilodha27 at gmail dot com>
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

#include "databaseserver.h"

// Qt includes

#include <QtGlobal>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>
#include <QStandardPaths>
#include <QThread>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DatabaseServer::Private
{
public:

    Private()
    {
        databaseProcess = 0;
        app             = 0;
        internalDBName  = QLatin1String("digikam");
    }

    QProcess*         databaseProcess;
    QString           internalDBName;
    QCoreApplication* app;
};

DatabaseServer::DatabaseServer(QCoreApplication* const application)
    : QThread(application),
      d(new Private)
{
    d->app = application;
}

DatabaseServer::~DatabaseServer()
{
    delete d;
}

void DatabaseServer::run()
{
    int waitTime = 1;

    // Loop to wait fro stopping the server.

    do
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Waiting " << waitTime << " seconds...";
        QThread::sleep(waitTime);
    }
    while (databaseServerStateEnum != stopped);

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Shutting down database server";
    emit done();
}

/**
 * Starts the database management server.
 */
bool DatabaseServer::startDatabaseProcess()
{
    DatabaseServerError error;

    if (DbEngineParameters::MySQLDatabaseType() == QLatin1String("QMYSQL"))
    {
        // return QVariant::fromValue(startMYSQLDatabaseProcess());
        error = startMYSQLDatabaseProcess();
        databaseServerStateEnum = running;
        return true;
    }
    else
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "This DB type is not supported.";
        DatabaseServerError errorDetails(DatabaseServerError::NotSupported, QString::fromUtf8("DBType is not supported."));
        error                   = errorDetails;
        databaseServerStateEnum = notRunning;
        return false;
    }
}

/**
 * Init and Starts Mysql server.
 */
DatabaseServerError DatabaseServer::startMYSQLDatabaseProcess()
{
    DatabaseServerError result;
    const QString dbType(DbEngineParameters::MySQLDatabaseType());
    DbEngineParameters internalServerParameters = DbEngineParameters::parametersFromConfig(KSharedConfig::openConfig(QLatin1String("digikamrc")));

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << internalServerParameters;

    const QString mysqldPath = internalServerParameters.internalServerMysqlServCmd;

    if ( mysqldPath.isEmpty() )
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql server comand set in configuration file!";
        return DatabaseServerError(DatabaseServerError::StartError, i18n("No path to mysql server comand set in configuration file!"));
    }

    // Create the database directories if they don't exists

    QString defaultAkDir = DbEngineParameters::internalServerPrivatePath();
    QString akDir        = defaultAkDir;

    if (internalServerParameters.internalServerPath().isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Internal Server data path is empty : we will use default path";
    }
    else if (internalServerParameters.internalServerPath() != defaultAkDir)
    {
        akDir = internalServerParameters.internalServerPath() + QLatin1String("/.mysql.digikam/");
    }

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Internal Server data path: " << akDir;

    const QString dataDir        = akDir        + QLatin1String("db_data");
    const QString miscDir        = defaultAkDir + QLatin1String("db_misc");
    const QString fileDataDir    = defaultAkDir + QLatin1String("file_db_data");

    const QString mysqldInitPath = internalServerParameters.internalServerMysqlInitCmd;

    if ( mysqldInitPath.isEmpty() )
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql initalization command set in configuration file!";
        return DatabaseServerError(DatabaseServerError::StartError, i18n("No path to mysql initalization command set in configuration file!."));
    }

    const QString mysqlInitCmd(QString::fromLatin1("%1 --user=%2 --datadir=%3")
                               .arg(mysqldInitPath)
                               .arg(getcurrentAccountUserName())
                               .arg(dataDir));

    if (!QFile::exists(defaultAkDir))
    {
        QDir().mkpath(defaultAkDir);
    }

    if (!QFile::exists(akDir))
    {
        QDir().mkpath(akDir);
    }

    if (!QFile::exists(dataDir))
    {
        QDir().mkpath(dataDir);
    }

    if (!QFile::exists(miscDir))
    {
        QDir().mkpath(miscDir);
    }

    if (!QFile::exists(fileDataDir))
    {
        QDir().mkpath(fileDataDir);
    }

    const QString globalConfig = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/database/mysql-global.conf"));
    const QString localConfig  = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/database/mysql-local.conf"));
    const QString actualConfig = defaultAkDir + QLatin1String("mysql.conf");

    if ( globalConfig.isEmpty() )
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot find MySQL server default configuration (mysql-global.conf)";

        return DatabaseServerError(DatabaseServerError::StartError, i18n("Cannot find MySQL server default configuration (mysql-global.conf)."));
    }

    bool confUpdate = false;
    QFile actualFile ( actualConfig );

    // Update conf only if either global (or local) is newer than actual

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

            qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;

            return DatabaseServerError(DatabaseServerError::StartError, str);
        }
    }

    // MySQL doesn't like world writeable config files (which makes sense), but
    // our config file somehow ends up being world-writable on some systems for no
    // apparent reason nevertheless, so fix that

    const QFile::Permissions allowedPerms = actualFile.permissions() &
                                            (QFile::ReadOwner | QFile::WriteOwner |
                                             QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther);

    if ( allowedPerms != actualFile.permissions() )
    {
        actualFile.setPermissions( allowedPerms );
    }

    if ( dataDir.isEmpty() )
    {
        QString  str = i18n("digiKam server was not able to create database data directory");
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    if ( akDir.isEmpty() )
    {
        QString  str = i18n("digiKam server was not able to create database log directory");
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    if ( miscDir.isEmpty() )
    {
        QString  str = i18n("digiKam server was not able to create database misc directory");
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    // Move mysql error log file out of the way

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
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to open MySQL error log.";
        }
    }

    // Clear mysql ib_logfile's in case innodb_log_file_size option changed in last confUpdate

    if ( confUpdate )
    {
        QFile(dataDir + QDir::separator() + QString::fromLatin1( "ib_logfile0" )).remove();
        QFile(dataDir + QDir::separator() + QString::fromLatin1( "ib_logfile1" )).remove();
    }

    // Synthesize the server command line arguments

    QStringList arguments;
    arguments << QString::fromLatin1( "--defaults-file=%1/mysql.conf" ).arg( defaultAkDir );
    arguments << QString::fromLatin1( "--datadir=%1/" ).arg( dataDir );
    arguments << QString::fromLatin1( "--socket=%1/mysql.socket" ).arg( miscDir );

    // Initialize the database

    if (!QFile(dataDir + QDir::separator() + QLatin1String("mysql")).exists())
    {
        QProcess initProcess;
        initProcess.start( mysqlInitCmd );

        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database initializer: " << initProcess.program() << initProcess.arguments();

        if ( !initProcess.waitForFinished() || initProcess.exitCode() != 0)
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << initProcess.readAllStandardOutput();

            QString  str = i18n("Could not start database init command.\n"
                                "Executable: %1\n"
                                "Process error:%2",
                                mysqlInitCmd,
                                initProcess.errorString());

            qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;

            return DatabaseServerError(DatabaseServerError::StartError, str);
        }
    }

    // Start the database server

    d->databaseProcess = new QProcess();
    d->databaseProcess->start( mysqldPath, arguments );

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database server: " << d->databaseProcess->program() << d->databaseProcess->arguments();

    if ( !d->databaseProcess->waitForStarted() || d->databaseProcess->exitCode() != 0)
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << d->databaseProcess->readAllStandardOutput();

        QString argumentStr = arguments.join(QLatin1String(", "));
        QString  str        = i18n("Could not start database server.");
        str                += i18n("<p>Executable: %1</p>",    mysqldPath);
        str                += i18n("<p>Arguments: %1</p>",     argumentStr);
        str                += i18n("<p>Process error: %1</p>", d->databaseProcess->errorString());

        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;

        delete d->databaseProcess;
        d->databaseProcess = 0;

        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    const QLatin1String initCon( "initConnection" );

    {
        QSqlDatabase db = QSqlDatabase::addDatabase( DbEngineParameters::MySQLDatabaseType(), initCon );
        db.setConnectOptions(QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir));
        db.setUserName(QLatin1String("root"));
        db.setDatabaseName( QString() ); // might not exist yet, then connecting to the actual db will fail

        if ( !db.isValid() )
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Invalid database object during database server startup";
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
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database process exited unexpectedly during initial connection!";
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "executable: "    << mysqldPath;
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "arguments: "     << arguments;
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "stdout: "        << d->databaseProcess->readAllStandardOutput();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "stderr: "        << d->databaseProcess->readAllStandardError();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "exit code: "     << d->databaseProcess->exitCode();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "process error: " << d->databaseProcess->errorString();

                QString str = i18n("Database process exited unexpectedly during initial connection."
                                   "<p>Executable: %1</p>”"
                                   "<p>Process error: %2</p>",
                                   mysqldPath,
                                   d->databaseProcess->errorString());

                return DatabaseServerError(DatabaseServerError::StartError, str);
            }
        }

        if ( opened )
        {
            {
                QSqlQuery query( db );

                if ( !query.exec( QString::fromLatin1( "USE %1" ).arg( d->internalDBName ) ) )
                {
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to use database" << d->internalDBName;
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Query error:"           << query.lastError().text();
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database error:"        << db.lastError().text();
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Trying to create database now...";

                    if ( !query.exec( QLatin1String( "CREATE DATABASE digikam" ) ) )
                    {
                        QString  str = i18n("Failed to create database"
                                            "<p>Query error: %1</p>"
                                            "<p>Database error: %2</p>",
                                            query.lastError().text(),
                                            db.lastError().text());

                        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;

                        return DatabaseServerError(DatabaseServerError::StartError, str);
                    }
                    else
                    {
                        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database was successfully created";
                    }
                }
            }

            // Make sure query is destroyed before we close the db

            db.close();
        }
    }

    QSqlDatabase::removeDatabase( initCon );

    databaseServerStateEnum = running;

    return result;
}

/**
 * Creates the initial database for the internal server instance.
 */
DatabaseServerError DatabaseServer::createDatabase()
{
    const QLatin1String initCon("initConnection");
    QSqlDatabase db = QSqlDatabase::addDatabase( QLatin1String("MYSQL"), initCon );

    // Might not exist yet, then connecting to the actual db will fail.

    db.setDatabaseName( QString() );

    if ( !db.isValid() )
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Invalid database object during initial database connection";
    }

    if ( db.open() )
    {
        QSqlQuery query( db );

        if ( !query.exec( QString::fromLatin1( "USE %1" ).arg( d->internalDBName ) ) )
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to use database" << d->internalDBName;
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Query error:"           << query.lastError().text();
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database error:"        << db.lastError().text();
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Trying to create database now...";

            if ( !query.exec( QLatin1String( "CREATE DATABASE digikam" ) ) )
            {
                QString str = i18n("Failed to create database"
                                   "<p>Query error: %1</p>"
                                   "<p>Database error: %2</p>",
                                   query.lastError().text(),
                                   db.lastError().text());

                qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;

                return DatabaseServerError(DatabaseServerError::StartError, str);
            }
            else
            {
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database was successfully created";
            }
        }

        // Make sure query is destroyed before we close the db

        db.close();
    }

    QSqlDatabase::removeDatabase( initCon );

    databaseServerStateEnum = started;

    return DatabaseServerError(DatabaseServerError::NoErrors, QString());
}

/**
 * Terminates the databaser server process.
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
    d->databaseProcess  = 0;
    d->app->exit(0);

    databaseServerStateEnum = stopped;
}

/**
 * Returns true if the server process is running.
 */
bool DatabaseServer::isRunning()
{
    if (d->databaseProcess == 0)
    {
        return false;
    }

    databaseServerStateEnum = running;

    return (d->databaseProcess->state() == QProcess::Running);
}

/**
 * Return the current user account name
 */
QString DatabaseServer::getcurrentAccountUserName() const
{
    QString name = QString::fromUtf8(qgetenv("USER"));   // Linux and OSX

    if (name.isEmpty())
        name = QString::fromUtf8(qgetenv("USERNAME"));   // Windows

    return name;
}

} // namespace Digikam
