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
#include <QObject>
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
#include "databaseserverstarter.h"

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

    QProcess*               databaseProcess;
    QString                 internalDBName;
    DatabaseServerStarter*  app; 
};

DatabaseServer::DatabaseServer(DatabaseServerStarter* const parent)
    : QThread(parent),
      d(new Private)
{
    d->app = parent;
    databaseServerStateEnum = started;
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

DatabaseServerError DatabaseServer::startDatabaseProcess()
{
    DatabaseServerError error;

    if (DbEngineParameters::MySQLDatabaseType() == QLatin1String("QMYSQL"))
    {
        error = startMYSQLDatabaseProcess();

        if (error.getErrorType() == DatabaseServerError::StartError)
        {
            databaseServerStateEnum = notRunning;
        }
        else
        {
            databaseServerStateEnum = running;
        }
    }
    else
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "This database type is not supported.";
        error                   = DatabaseServerError(DatabaseServerError::NotSupported, QString::fromUtf8("Database type is not supported."));
        databaseServerStateEnum = notRunning;
    }

    return error;
}

DatabaseServerError DatabaseServer::startMYSQLDatabaseProcess()
{
    DatabaseServerError result;
    const QString dbType(DbEngineParameters::MySQLDatabaseType());
    DbEngineParameters internalServerParameters = DbEngineParameters::parametersFromConfig(KSharedConfig::openConfig(QLatin1String("digikamrc")));

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << internalServerParameters;

    const QString mysqldCmd = internalServerParameters.internalServerMysqlServCmd;

    if (mysqldCmd.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql server command set in configuration file!";
        return DatabaseServerError(DatabaseServerError::StartError, i18n("No path to mysql server command set in configuration file!"));
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

    if (mysqldInitPath.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql initialization command set in configuration file!";
        return DatabaseServerError(DatabaseServerError::StartError, i18n("No path to mysql initialization command set in configuration file!."));
    }

    if (!QFile::exists(defaultAkDir))
    {
        if (!QDir().mkpath(defaultAkDir))
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory " << defaultAkDir;
            return DatabaseServerError(DatabaseServerError::StartError, i18n("Cannot create directory %1", QDir::toNativeSeparators(defaultAkDir)));
        }
    }

    if (!QFile::exists(akDir))
    {
        if (!QDir().mkpath(akDir))
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory " << akDir;
            return DatabaseServerError(DatabaseServerError::StartError, i18n("Cannot create directory %1", QDir::toNativeSeparators(akDir)));
        }
    }

    if (!QFile::exists(dataDir))
    {
        if (!QDir().mkpath(dataDir))
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory " << dataDir;
            return DatabaseServerError(DatabaseServerError::StartError, i18n("Cannot create directory %1", QDir::toNativeSeparators(dataDir)));
        }
    }

    if (!QFile::exists(miscDir))
    {
        if (!QDir().mkpath(miscDir))
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory " << miscDir;
            return DatabaseServerError(DatabaseServerError::StartError, i18n("Cannot create directory %1", QDir::toNativeSeparators(miscDir)));
        }
    }

    if (!QFile::exists(fileDataDir))
    {
        if (!QDir().mkpath(fileDataDir))
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory " << fileDataDir;
            return DatabaseServerError(DatabaseServerError::StartError, i18n("Cannot create directory %1", QDir::toNativeSeparators(fileDataDir)));
        }
    }

#ifdef Q_OS_WIN
    const QString globalConfig = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/database/mysql-windows.conf"));
#else
    const QString globalConfig = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/database/mysql-global.conf"));
#endif

    const QString localConfig  = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/database/mysql-local.conf"));
    const QString actualConfig = defaultAkDir + QLatin1String("mysql.conf");

    if (globalConfig.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot find MySQL server default configuration (mysql-global.conf)";

        return DatabaseServerError(DatabaseServerError::StartError, i18n("Cannot find MySQL server default configuration (mysql-global.conf)."));
    }

    bool confUpdate = false;
    QFile actualFile (actualConfig);

    // Update conf only if either global (or local) is newer than actual

    if ((QFileInfo(globalConfig).lastModified() > QFileInfo(actualFile).lastModified()) ||
        (QFileInfo(localConfig).lastModified()  > QFileInfo(actualFile).lastModified()))
    {
        QFile globalFile(globalConfig);
        QFile localFile (localConfig);

        if (globalFile.open(QFile::ReadOnly) && actualFile.open(QFile::WriteOnly))
        {
            actualFile.write(globalFile.readAll());

            if (!localConfig.isEmpty())
            {
                if (localFile.open(QFile::ReadOnly))
                {
                    actualFile.write(localFile.readAll());
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

    if (allowedPerms != actualFile.permissions())
    {
        actualFile.setPermissions(allowedPerms);
    }

    if (dataDir.isEmpty())
    {
        QString  str = i18n("digiKam server was not able to create database data directory");
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    if (akDir.isEmpty())
    {
        QString  str = i18n("digiKam server was not able to create database log directory");
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    if (miscDir.isEmpty())
    {
        QString  str = i18n("digiKam server was not able to create database misc directory");
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;
        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    // Move mysql error log file out of the way

    const QFileInfo errorLog(dataDir + QLatin1Char('/') + QString::fromLatin1("mysql.err"));

    if (errorLog.exists())
    {
        QFile logFile(errorLog.absoluteFilePath());
        QFile oldLogFile(dataDir + QLatin1Char('/') + QString::fromLatin1("mysql.err.old"));

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

    if (confUpdate)
    {
        QFile(dataDir + QLatin1Char('/') + QString::fromLatin1("ib_logfile0")).remove();
        QFile(dataDir + QLatin1Char('/') + QString::fromLatin1("ib_logfile1")).remove();
    }

    // Synthesize the server initialization command line arguments

    QStringList mysqlInitCmdArgs;
    mysqlInitCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--datadir=%1").arg(dataDir));

    // Synthesize the server command line arguments

    QStringList mysqldCmdArgs;
    mysqldCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--defaults-file=%1").arg(actualConfig));
    mysqldCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--datadir=%1").arg(dataDir));

#ifdef Q_OS_WIN
    mysqldCmdArgs << QString::fromLatin1("--port=3307");
#else
    mysqldCmdArgs << QString::fromLatin1("--socket=%1/mysql.socket").arg(miscDir);
#endif

    // Initialize the database

    if (!QFile(dataDir + QLatin1Char('/') + QLatin1String("mysql")).exists())
    {
        QProcess initProcess;
        initProcess.start(mysqldInitPath, mysqlInitCmdArgs);

        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database initializer: " << initProcess.program() << initProcess.arguments();

        if (!initProcess.waitForFinished() || initProcess.exitCode() != 0)
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << initProcess.readAllStandardOutput();

            QString  str = i18n("Could not start database initializer.\n"
                                "Executable: %1\n"
                                "Arguments: %2\n"
                                "Process error: %3",
                                initProcess.program(),
                                initProcess.arguments().join(QLatin1Char(',')),
                                initProcess.errorString());

            qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;

            return DatabaseServerError(DatabaseServerError::StartError, str);
        }
    }

    // Start the database server

    d->databaseProcess = new QProcess();
    d->databaseProcess->start(mysqldCmd, mysqldCmdArgs);

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database server: " << d->databaseProcess->program() << d->databaseProcess->arguments();

    if (!d->databaseProcess->waitForStarted() || d->databaseProcess->exitCode() != 0)
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << d->databaseProcess->readAllStandardOutput();

        QString argumentStr = mysqldCmdArgs.join(QLatin1String(", "));
        QString  str        = i18n("Could not start database server.");
        str                += i18n("<p>Executable: %1</p>",    d->databaseProcess->program());
        str                += i18n("<p>Arguments: %1</p>",     d->databaseProcess->arguments().join(QLatin1Char(',')));
        str                += i18n("<p>Process error: %1</p>", d->databaseProcess->errorString());

        qCDebug(DIGIKAM_DATABASESERVER_LOG) << str;

        delete d->databaseProcess;
        d->databaseProcess = 0;

        return DatabaseServerError(DatabaseServerError::StartError, str);
    }

    const QLatin1String initCon("initConnection");

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(DbEngineParameters::MySQLDatabaseType(), initCon);

#ifdef Q_OS_WIN
        db.setHostName(QLatin1String("localhost"));
        db.setPort(3307);
#else
        db.setConnectOptions(QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir));
#endif

        db.setUserName(QLatin1String("root"));
        db.setDatabaseName(QString()); // might not exist yet, then connecting to the actual db will fail

        if (!db.isValid())
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Invalid database object during database server startup";
        }

        bool opened = false;

        for (int i = 0; i < 120; ++i)
        {
            opened = db.open();

            if (opened)
            {
                break;
            }

            if (d->databaseProcess->waitForFinished(500))
            {
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database process exited unexpectedly during initial connection!";
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "executable: "    << d->databaseProcess->program();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "arguments: "     << d->databaseProcess->arguments().join(QLatin1Char(','));
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "stdout: "        << d->databaseProcess->readAllStandardOutput();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "stderr: "        << d->databaseProcess->readAllStandardError();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "exit code: "     << d->databaseProcess->exitCode();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "process error: " << d->databaseProcess->errorString();

                QString str = i18n("Database process exited unexpectedly during initial connection."
                                   "<p>Executable: %1</p>”"
                                   "<p>Arguments: %2</p>”"
                                   "<p>Process error: %3</p>",
                                   d->databaseProcess->program(),
                                   d->databaseProcess->arguments().join(QLatin1Char(',')),
                                   d->databaseProcess->errorString());

                return DatabaseServerError(DatabaseServerError::StartError, str);
            }
        }

        if (opened)
        {
            {
                QSqlQuery query(db);

                if (!query.exec(QString::fromLatin1("USE %1").arg(d->internalDBName)))
                {
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to use database" << d->internalDBName;
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Query error:"           << query.lastError().text();
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database error:"        << db.lastError().text();
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Trying to create database now...";

                    if (!query.exec(QLatin1String("CREATE DATABASE digikam")))
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

    QSqlDatabase::removeDatabase(initCon);

    databaseServerStateEnum = running;

    return result;
}

DatabaseServerError DatabaseServer::createDatabase()
{
    const QLatin1String initCon("initConnection");
    QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("MYSQL"), initCon);

    // Might not exist yet, then connecting to the actual db will fail.

    db.setDatabaseName(QString());

    if (!db.isValid())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Invalid database object during initial database connection";
    }

    if (db.open())
    {
        QSqlQuery query(db);

        if (!query.exec(QString::fromLatin1("USE %1").arg(d->internalDBName)))
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to use database" << d->internalDBName;
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Query error:"           << query.lastError().text();
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database error:"        << db.lastError().text();
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Trying to create database now...";

            if (!query.exec(QLatin1String("CREATE DATABASE digikam")))
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

    QSqlDatabase::removeDatabase(initCon);

    databaseServerStateEnum = started;

    return DatabaseServerError(DatabaseServerError::NoErrors, QString());
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
    d->databaseProcess  = 0;

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

QString DatabaseServer::getcurrentAccountUserName() const
{
    QString name = QString::fromUtf8(qgetenv("USER"));   // Linux and OSX

    if (name.isEmpty())
        name = QString::fromUtf8(qgetenv("USERNAME"));   // Windows

    return name;
}

} // namespace Digikam
