/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : Mysql internal database server
 *
 * Copyright (C) 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtGlobal>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dbengineparameters.h"
#include "databaseserverstarter.h"

namespace Digikam
{

class DatabaseServer::Private
{
public:

    Private()
    {
        app             = 0;
        databaseProcess = 0;
    }

    DbEngineParameters     params;
    DatabaseServerStarter* app;
    QProcess*              databaseProcess;

    QString                internalDBName;
    QString                mysqldInitPath;
    QString                mysqldCmd;
    QString                dataDir;
    QString                miscDir;
    QString                fileDataDir;
    QString                actualConfig;
};

DatabaseServer::DatabaseServer(const DbEngineParameters& params, DatabaseServerStarter* const parent)
    : QThread(parent),
      d(new Private)
{
    d->app    = parent;
    d->params = params;

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << d->params;

    QString defaultAkDir = DbEngineParameters::internalServerPrivatePath();
    QString dataDir;

    if (d->params.internalServerPath().isEmpty())
    {
        dataDir = QDir(defaultAkDir).absoluteFilePath(QLatin1String("db_data"));
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No internal server data path is given, we will use the default." << dataDir;
    }
    else
    {
        dataDir = QDir(d->params.internalServerPath()).absoluteFilePath(QLatin1String(".mysql.digikam/db_data"));
    }

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Internal Server data path:" << dataDir;

    d->internalDBName       = QLatin1String("digikam");
    d->mysqldInitPath       = d->params.internalServerMysqlInitCmd;
    d->mysqldCmd            = d->params.internalServerMysqlServCmd;
    d->dataDir              = dataDir;
    d->miscDir              = QDir(defaultAkDir).absoluteFilePath(QLatin1String("db_misc"));
    d->fileDataDir          = QDir(defaultAkDir).absoluteFilePath(QLatin1String("file_db_data"));
    d->actualConfig         = QDir(defaultAkDir).absoluteFilePath(QLatin1String("mysql.conf"));
    databaseServerStateEnum = started;
}

DatabaseServer::~DatabaseServer()
{
    delete d;
}

void DatabaseServer::run()
{
    quint64 runningTime = 0;
    int debugTime       = 0;
    int waitTime        = 1;

    // Loop to wait for stopping the server.

    do
    {
        if (!debugTime)
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Running" << runningTime << "seconds...";
            debugTime = 30;
        }

        QThread::sleep(waitTime);
        runningTime++;
        debugTime--;
    }
    while (databaseServerStateEnum != stopped);

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Shutting down database server";
    emit done();
}

DatabaseServerError DatabaseServer::startDatabaseProcess()
{
    DatabaseServerError error;

    if (d->params.isMySQL())
    {
        error = startMysqlDatabaseProcess();
    }
    else
    {
        error = DatabaseServerError(DatabaseServerError::NotSupported,
                                    i18n("Database type is not supported."));
    }

    if (error.getErrorType() == DatabaseServerError::StartError)
    {
        databaseServerStateEnum = notRunning;
    }
    else if (error.getErrorType() == DatabaseServerError::NotSupported)
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "This database type is not supported.";
        databaseServerStateEnum = notRunning;
    }
    else
    {
        databaseServerStateEnum = running;
    }

    return error;
}

void DatabaseServer::stopDatabaseProcess()
{
    if (!d->databaseProcess)
    {
        return;
    }

    d->databaseProcess->terminate();

    if (d->databaseProcess->state() == QProcess::Running && !d->databaseProcess->waitForFinished(5000))
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database process will be killed now";
        d->databaseProcess->kill();
        d->databaseProcess->waitForFinished();
    }

    d->databaseProcess->~QProcess();
    d->databaseProcess = 0;

    databaseServerStateEnum = stopped;
    wait();
}

bool DatabaseServer::isRunning() const
{
    if (d->databaseProcess == 0)
    {
        return false;
    }

    return (databaseServerStateEnum == running);
}

DatabaseServerError DatabaseServer::startMysqlDatabaseProcess()
{
    DatabaseServerError result;

    DatabaseServerError error = checkDatabaseDirs();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    error = initMysqlConfig();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    removeMysqlLogs();

    error = createMysqlFiles();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    error = startMysqlServer();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    error = initMysqlDatabase();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    databaseServerStateEnum = running;

    return result;
}

DatabaseServerError DatabaseServer::checkDatabaseDirs() const
{
    DatabaseServerError result;

    if (d->mysqldCmd.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql server command set in configuration file!";

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("No path to mysql server command set "
                                        "in configuration file!"));
    }

    if (d->mysqldInitPath.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql initialization command set in configuration file!";

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("No path to mysql initialization "
                                        "command set in configuration file!."));
    }

    if (!QFile::exists(d->dataDir) && !QDir().mkpath(d->dataDir))
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory "
                                            << d->dataDir;

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("Cannot create directory %1",
                                        QDir::toNativeSeparators(d->dataDir)));
    }

    if (!QFile::exists(d->miscDir) && !QDir().mkpath(d->miscDir))
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory "
                                            << d->miscDir;

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("Cannot create directory %1",
                                        QDir::toNativeSeparators(d->miscDir)));
    }

    if (!QFile::exists(d->fileDataDir) && !QDir().mkpath(d->fileDataDir))
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory "
                                            << d->fileDataDir;

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("Cannot create directory %1",
                                        QDir::toNativeSeparators(d->fileDataDir)));
    }

    return result;
}

DatabaseServerError DatabaseServer::initMysqlConfig() const
{
    DatabaseServerError result;

    QString globalConfig = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  QLatin1String("digikam/database/mysql-global.conf"));
    QString localConfig  = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  QLatin1String("digikam/database/mysql-local.conf"));

    if (!globalConfig.isEmpty())
    {
        bool confUpdate = false;
        bool confShouldUpdate = false;
        QFile actualFile(d->actualConfig);

        // Update actualconf only if either global or local is newer than actual
        // If actual does not yet exist it was initialised with datetime 0
        // so it will get updated too

        if ((QFileInfo(globalConfig).lastModified() > QFileInfo(actualFile).lastModified()) ||
            (QFileInfo(localConfig).lastModified()  > QFileInfo(actualFile).lastModified()))
        {
            confShouldUpdate = true;

            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "The mysql configuration is outdated,"
                                                << d->actualConfig
                                                << "will be updated.";

            QFile globalFile(globalConfig);
            QFile localFile (localConfig);

            if (globalFile.open(QFile::ReadOnly) && actualFile.open(QFile::WriteOnly))
            {
                actualFile.write(globalFile.readAll());
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Updated mysql configuration with" << globalConfig;

                if (!localConfig.isEmpty() && localFile.open(QFile::ReadOnly))
                {
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Updated mysql configuration with" << localConfig;
                    actualFile.write(localFile.readAll());
                    localFile.close();
                }

                globalFile.close();
                actualFile.close();

                confUpdate = true;
            }
        }

        // MySQL doesn't like world writeable config files (which makes sense), but
        // our config file somehow ends up being world-writable on some systems for no
        // apparent reason nevertheless, so fix that

        if (confUpdate)
        {
            const QFile::Permissions allowedPerms
                = actualFile.permissions()
                  & (QFile::ReadOwner | QFile::WriteOwner |
                     QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther);

            if (allowedPerms != actualFile.permissions())
            {
                actualFile.setPermissions(allowedPerms);
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Fixed permissions of mysql configuration file.";
            }
        }
        else if (confShouldUpdate)
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Unable to create MySQL server configuration file."
                                                << "This means that either the default configuration file"
                                                << globalConfig
                                                << "was not readable or the target file"
                                                << d->actualConfig
                                                << "could not be written.";

            QString errorMsg = i18n("Unable to create MySQL server configuration file."
                                    "<p>This means that either the default configuration file</p>"
                                    "<p>%1</p>"
                                    "<p>was not readable or the target file</p>"
                                    "<p>%2</p>"
                                    "<p>could not be written.</p>",
                                    globalConfig, d->actualConfig);

            return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
        }
        else
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "The mysql configuration was already up-to-date:"
                                                << d->actualConfig;
        }
    }
    else
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot find MySQL server default configuration (mysql-global.conf)";

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("Cannot find MySQL server default "
                                        "configuration (mysql-global.conf)."));
    }

    return result;
}

void DatabaseServer::removeMysqlLogs() const
{
    // Move mysql error log file out of the way

    const QFileInfo errorLog(d->dataDir,
                             QLatin1String("mysql.err"));

    if (errorLog.exists())
    {
        QFile logFile(errorLog.absoluteFilePath());
        QFile oldLogFile(QDir(d->dataDir).absoluteFilePath(QLatin1String("mysql.err.old")));

        if (logFile.open(QFile::ReadOnly) && oldLogFile.open(QFile::Append))
        {
            oldLogFile.write(logFile.readAll());
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

    QFile(QDir(d->dataDir).absoluteFilePath(QLatin1String("ib_logfile0"))).remove();
    QFile(QDir(d->dataDir).absoluteFilePath(QLatin1String("ib_logfile1"))).remove();
}

DatabaseServerError DatabaseServer::createMysqlFiles() const
{
    DatabaseServerError result;

    // Initialize the database

    if (!QFile(QDir(d->dataDir).absoluteFilePath(QLatin1String("mysql"))).exists())
    {
        // Synthesize the server initialization command line arguments

        QStringList mysqlInitCmdArgs;
        mysqlInitCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--datadir=%1").arg(d->dataDir));

#ifndef Q_OS_WIN
        mysqlInitCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--defaults-file=%1").arg(d->actualConfig));
#endif

        QProcess initProcess;
        initProcess.setProcessEnvironment(adjustedEnvironmentForAppImage());
        initProcess.start(d->mysqldInitPath, mysqlInitCmdArgs);

        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database initializer:"
                                            << initProcess.program()
                                            << initProcess.arguments();

        if (!initProcess.waitForFinished() || initProcess.exitCode() != 0)
        {
            return DatabaseServerError(DatabaseServerError::StartError,
                                       processErrorLog(&initProcess,
                                                       i18n("Could not start database initializer.")));
        }
    }

    return result;
}

DatabaseServerError DatabaseServer::startMysqlServer()
{
    DatabaseServerError result;

    // Synthesize the server command line arguments

    QStringList mysqldCmdArgs;
    mysqldCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--defaults-file=%1").arg(d->actualConfig))
                  << QDir::toNativeSeparators(QString::fromLatin1("--datadir=%1").arg(d->dataDir));

#ifdef Q_OS_WIN
    mysqldCmdArgs << QLatin1String("--skip-networking=0")
                  << QLatin1String("--port=3307");
#else
    mysqldCmdArgs << QString::fromLatin1("--socket=%1/mysql.socket").arg(d->miscDir);
#endif

    // Start the database server

    d->databaseProcess = new QProcess();
    d->databaseProcess->setProcessEnvironment(adjustedEnvironmentForAppImage());
    d->databaseProcess->start(d->mysqldCmd, mysqldCmdArgs);

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database server:"
                                        << d->databaseProcess->program()
                                        << d->databaseProcess->arguments();

    if (!d->databaseProcess->waitForStarted() || d->databaseProcess->exitCode() != 0)
    {
        QString errorMsg = processErrorLog(d->databaseProcess,
                                           i18n("Could not start database server."));

        delete d->databaseProcess;
        d->databaseProcess = 0;

        return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
    }

    return result;
}

DatabaseServerError DatabaseServer::initMysqlDatabase() const
{
    DatabaseServerError result;

    const QLatin1String initCon("initConnection");

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(DbEngineParameters::MySQLDatabaseType(), initCon);

#ifdef Q_OS_WIN
        db.setHostName(QLatin1String("localhost"));
        db.setPort(3307);
#else
        db.setConnectOptions(QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(d->miscDir));
#endif

        db.setUserName(QLatin1String("root"));

        // might not exist yet, then connecting to the actual db will fail
        db.setDatabaseName(QString());

        if (!db.isValid())
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Invalid database object during database server startup";

            return DatabaseServerError(DatabaseServerError::StartError,
                                       i18n("Invalid database object during database "
                                            "server startup"));
        }

        bool opened = false;
        bool exited = false;

        for (int i = 0; i < 120; ++i)
        {
            opened = db.open();

            if (opened)
            {
                break;
            }

            if (d->databaseProcess->waitForFinished(500))
            {
                exited = true;
                break;
            }
        }

        if (!opened)
        {
            QString firstLine;
            QString errorMsg;

            if (exited)
            {
                errorMsg = processErrorLog(d->databaseProcess,
                                           i18n("Database process exited unexpectedly "
                                                "during initial connection."));
            }
            else
            {
                errorMsg = processErrorLog(d->databaseProcess,
                                           i18n("Could not connect to Database after "
                                                "trying for 60 seconds."));
            }

            return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
        }

        QSqlQuery query(db);

        if (!query.exec(QString::fromLatin1("USE %1").arg(d->internalDBName)))
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to use database"
                                                << d->internalDBName;
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Query error:"
                                                << query.lastError().text();
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database error:"
                                                << db.lastError().text();
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Trying to create database now";

            if (query.exec(QLatin1String("CREATE DATABASE digikam")))
            {
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database was successfully created";
            }
            else
            {
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to create database";
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Query error:"
                                                    << query.lastError().text();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database error:"
                                                    << db.lastError().text();

                QString  errorMsg = i18n("Failed to create database"
                                         "<p>Query error: %1</p>"
                                         "<p>Database error: %2</p>",
                                         query.lastError().text(),
                                         db.lastError().text());

                return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
            }

            // Make sure query is destroyed before we close the db

            db.close();
        }
    }

    QSqlDatabase::removeDatabase(initCon);

    return result;
}

QString DatabaseServer::getcurrentAccountUserName() const
{
    QString name = QString::fromUtf8(qgetenv("USER"));   // Linux and OSX

    if (name.isEmpty())
    {
        name = QString::fromUtf8(qgetenv("USERNAME"));   // Windows
    }

    return name;
}

QString DatabaseServer::processErrorLog(QProcess* const process, const QString& msg) const
{
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << msg;
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Executable:"
                                        << process->program();
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Arguments:"
                                        << process->arguments().join(QLatin1String(", "));
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Stdout:"
                                        << process->readAllStandardOutput();
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Stderr:"
                                        << process->readAllStandardError();
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Exit code:"
                                        << process->exitCode();
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Process error:"
                                        << process->errorString();

    return i18n("%1"
                "<p>Executable: %2</p>"
                "<p>Arguments: %3</p>"
                "<p>Process error: %4</p>",
                msg,
                process->program(),
                process->arguments().join(QLatin1String(", ")),
                process->errorString());
}

} // namespace Digikam
