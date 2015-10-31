/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for database connection parameters.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Holger Foerster <hamsi2k at freenet dot de>
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

/*
#ifndef DATABASEPARAMETERS_DEBUG
#define DATABASEPARAMETERS_DEBUG
#endif
*/

#include "digikam_config.h"
#include "databaseparameters.h"

// Qt includes

#include <QDir>
#include <QUrlQuery>
#include <QFile>
#include <QCryptographicHash>
#include <QStandardPaths>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"

namespace
{
static const char* configGroupDatabase          = "Database Settings";
static const char* configInternalDatabaseServer = "Internal Database Server";
static const char* configDatabaseType           = "Database Type";
static const char* configDatabaseName           = "Database Name";
static const char* configDatabaseNameThumbnails = "Database Name Thumbnails";
static const char* configDatabaseHostName       = "Database Hostname";
static const char* configDatabasePort           = "Database Port";
static const char* configDatabaseUsername       = "Database Username";
static const char* configDatabasePassword       = "Database Password";
static const char* configDatabaseConnectOptions = "Database Connectoptions";
// legacy
static const char* configDatabaseFilePathEntry  = "Database File Path";
static const char* configAlbumPathEntry         = "Album Path";

static const char* digikam4db                   = "digikam4.db";
static const char* thumbnails_digikamdb         = "thumbnails-digikam.db";
}

namespace Digikam
{

DatabaseParameters::DatabaseParameters()
    : port(-1), internalServer(false)
{
}

DatabaseParameters::DatabaseParameters(const QString& type,
                                       const QString& databaseName,
                                       const QString& connectOptions,
                                       const QString& hostName,
                                       int port,
                                       bool internalServer,
                                       const QString& userName,
                                       const QString& password,
                                       const QString& databaseNameThumbnails)
    : databaseType(type),
      databaseName(databaseName),
      connectOptions(connectOptions),
      hostName(hostName),
      port(port),
      internalServer(internalServer),
      userName(userName),
      password(password),
      databaseNameThumbnails(databaseNameThumbnails)
{
}

DatabaseParameters::DatabaseParameters(const QUrl& url)
    : port(-1),
      internalServer(false)
{
    databaseType           = QUrlQuery(url).queryItemValue(QLatin1String("databaseType"));
    databaseName           = QUrlQuery(url).queryItemValue(QLatin1String("databaseName"));
    databaseNameThumbnails = QUrlQuery(url).queryItemValue(QLatin1String("databaseNameThumbnails"));
    connectOptions         = QUrlQuery(url).queryItemValue(QLatin1String("connectOptions"));
    hostName               = QUrlQuery(url).queryItemValue(QLatin1String("hostName"));
    QString queryPort      = QUrlQuery(url).queryItemValue(QLatin1String("port"));

    if (!queryPort.isNull())
    {
        port = queryPort.toInt();
    }

#if defined(HAVE_MYSQLSUPPORT) && defined(HAVE_INTERNALMYSQL)
    QString queryServer = QUrlQuery(url).queryItemValue(QLatin1String("internalServer"));

    if (!queryServer.isNull())
    {
        internalServer = (queryServer == QLatin1String("true"));
    }
#else
    internalServer = false;
#endif

    userName       = QUrlQuery(url).queryItemValue(QLatin1String("userName"));
    password       = QUrlQuery(url).queryItemValue(QLatin1String("password"));
}

bool DatabaseParameters::operator==(const DatabaseParameters& other) const
{
    return(databaseType   == other.databaseType                   &&
           databaseName   == other.databaseName                   &&
           databaseNameThumbnails == other.databaseNameThumbnails &&
           connectOptions == other.connectOptions                 &&
           hostName       == other.hostName                       &&
           port           == other.port                           &&
           internalServer == other.internalServer                 &&
           userName       == other.userName                       &&
           password       == other.password);
}

bool DatabaseParameters::operator!=(const DatabaseParameters& other) const
{
    return (!operator == (other));
}

bool DatabaseParameters::isValid() const
{
    if (isSQLite())
    {
        return !databaseName.isEmpty();
    }

    return false;
}

bool DatabaseParameters::isSQLite() const
{
    return (databaseType == QLatin1String("QSQLITE"));
}

bool DatabaseParameters::isMySQL() const
{
    return (databaseType == QLatin1String("QMYSQL"));
}

QString DatabaseParameters::SQLiteDatabaseType()
{
    return QLatin1String("QSQLITE");
}

QString DatabaseParameters::MySQLDatabaseType()
{
    return QLatin1String("QMYSQL");
}

QString DatabaseParameters::SQLiteDatabaseFile() const
{
    if (isSQLite())
    {
        return databaseName;
    }

    return QString();
}

QByteArray DatabaseParameters::hash() const
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(databaseType.toUtf8());
    md5.addData(databaseName.toUtf8());
    md5.addData(connectOptions.toUtf8());
    md5.addData(hostName.toUtf8());
    md5.addData((const char*)&port, sizeof(int));
    md5.addData(userName.toUtf8());
    md5.addData(password.toUtf8());
    return md5.result().toHex();
}

DatabaseParameters DatabaseParameters::parametersFromConfig(KSharedConfig::Ptr config, const QString& configGroup)
{
    DatabaseParameters parameters;
    parameters.readFromConfig(config, configGroup);
    return parameters;
}

void DatabaseParameters::readFromConfig(KSharedConfig::Ptr config, const QString& configGroup)
{
    KConfigGroup group;

    if (configGroup.isNull())
    {
        group = config->group(configGroupDatabase);
    }
    else
    {
        group = config->group(configGroup);
    }

    databaseType             = group.readEntry(configDatabaseType, QString());

    if (isSQLite()) // see bug #267131
    {
        databaseName           = group.readPathEntry(configDatabaseName, QString());
        databaseNameThumbnails = group.readPathEntry(configDatabaseNameThumbnails, QString());
    }
    else
    {
        databaseName           = group.readEntry(configDatabaseName, QString());
        databaseNameThumbnails = group.readEntry(configDatabaseNameThumbnails, QString());
    }

    hostName                 = group.readEntry(configDatabaseHostName, QString());
    port                     = group.readEntry(configDatabasePort, -1);
    userName                 = group.readEntry(configDatabaseUsername, QString());
    password                 = group.readEntry(configDatabasePassword, QString());
    connectOptions           = group.readEntry(configDatabaseConnectOptions, QString());
#if defined(HAVE_MYSQLSUPPORT) && defined(HAVE_INTERNALMYSQL)
    internalServer           = group.readEntry(configInternalDatabaseServer, false);
#else
    internalServer           = false;
#endif

    if (isSQLite() && !databaseName.isNull())
    {
        QString orgName = databaseName;
        setDatabasePath(orgName);
        setThumbsDatabasePath(orgName);
    }
}

void DatabaseParameters::setDatabasePath(const QString& folderOrFileOrName)
{
    if (isSQLite())
    {
        databaseName = databaseFileSQLite(folderOrFileOrName);
    }
    else
    {
        databaseName = folderOrFileOrName;
    }
}

void DatabaseParameters::setThumbsDatabasePath(const QString& folderOrFileOrName)
{
    if (isSQLite())
    {
        databaseNameThumbnails = thumbnailDatabaseFileSQLite(folderOrFileOrName);
    }
    else
    {
        databaseNameThumbnails = folderOrFileOrName;
    }
}

QString DatabaseParameters::databaseFileSQLite(const QString& folderOrFile)
{
    QFileInfo fileInfo(folderOrFile);

    if (fileInfo.isDir())
    {
        return QDir::cleanPath(fileInfo.filePath() + QDir::separator() + QLatin1String(digikam4db));
    }

    return QDir::cleanPath(folderOrFile);
}

QString DatabaseParameters::thumbnailDatabaseFileSQLite(const QString& folderOrFile)
{
    QFileInfo fileInfo(folderOrFile);

    if (fileInfo.isDir())
    {
        return QDir::cleanPath(fileInfo.filePath() + QDir::separator() + QLatin1String(thumbnails_digikamdb));
    }

    return QDir::cleanPath(folderOrFile);
}

void DatabaseParameters::legacyAndDefaultChecks(const QString& suggestedPath, KSharedConfig::Ptr config)
{
    // Additional semantic checks for the database section.
    // If the internal server should be started, then the connection options must be reset
    if (databaseType == QLatin1String("QMYSQL") && internalServer)
    {
        const QString miscDir  = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("digikam/db_misc");
        databaseType           = QLatin1String("QMYSQL");
        databaseName           = QLatin1String("digikam");
        internalServer         = true;
        databaseNameThumbnails = QLatin1String("digikam");
        hostName.clear();
        port                   = -1;
        userName               = QLatin1String("root");
        password.clear();
        connectOptions         = QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir);
    }

    if (databaseType.isEmpty())
    {
        // Empty 1.3 config: migration from older versions
        KConfigGroup group  = config->group("Album Settings");

        QString databaseFilePath;

        if (group.hasKey(configDatabaseFilePathEntry))
        {
            // 1.0 - 1.2 style database file path?
            databaseFilePath = group.readEntry(configDatabaseFilePathEntry, QString());
        }
        else if (group.hasKey(configAlbumPathEntry))
        {
            // <= 0.9 style album path entry?
            databaseFilePath = group.readEntry(configAlbumPathEntry, QString());
        }
        else if (!suggestedPath.isNull())
        {
            databaseFilePath = suggestedPath;
        }


        if (!databaseFilePath.isEmpty())
        {
            *this = parametersForSQLite(databaseFileSQLite(databaseFilePath));
        }

        // Be aware that schema updating from  <= 0.9 requires reading the "Album Path", so do not remove it here
    }
}

void DatabaseParameters::removeLegacyConfig(KSharedConfig::Ptr config)
{
    KConfigGroup group  = config->group("Album Settings");

    if (group.hasKey(configDatabaseFilePathEntry))
    {
        group.deleteEntry(configDatabaseFilePathEntry);
    }

    if (group.hasKey(configAlbumPathEntry))
    {
        group.deleteEntry(configAlbumPathEntry);
    }
}

void DatabaseParameters::writeToConfig(KSharedConfig::Ptr config, const QString& configGroup) const
{
    KConfigGroup group;

    if (configGroup.isNull())
    {
        group = config->group(configGroupDatabase);
    }
    else
    {
        group = config->group(configGroup);
    }

    QString dbName       = getDatabaseNameOrDir();
    QString dbNameThumbs = getThumbsDatabaseNameOrDir();

    group.writeEntry(configDatabaseType,           databaseType);
    group.writeEntry(configDatabaseName,           dbName);
    group.writeEntry(configDatabaseNameThumbnails, dbNameThumbs);
    group.writeEntry(configDatabaseHostName,       hostName);
    group.writeEntry(configDatabasePort,           port);
    group.writeEntry(configDatabaseUsername,       userName);
    group.writeEntry(configDatabasePassword,       password);
    group.writeEntry(configDatabaseConnectOptions, connectOptions);
    group.writeEntry(configInternalDatabaseServer, internalServer);
}

QString DatabaseParameters::getDatabaseNameOrDir() const
{
    if (isSQLite())
    {
        return databaseDirectorySQLite(databaseName);
    }

    return databaseName;
}

QString DatabaseParameters::getThumbsDatabaseNameOrDir() const
{
    if (isSQLite())
    {
        return thumbnailDatabaseDirectorySQLite(databaseNameThumbnails);
    }

    return databaseNameThumbnails;
}

QString DatabaseParameters::databaseDirectorySQLite(const QString& path)
{
    if (path.endsWith(QLatin1String(digikam4db)))
    {
        QString chopped(path);
        chopped.chop(QString(QLatin1String(digikam4db)).length());
        return chopped;
    }

    return path;
}

QString DatabaseParameters::thumbnailDatabaseDirectorySQLite(const QString& path)
{
    if (path.endsWith(QLatin1String(thumbnails_digikamdb)))
    {
        QString chopped(path);
        chopped.chop(QString(QLatin1String(thumbnails_digikamdb)).length());
        return chopped;
    }

    return path;
}

DatabaseParameters DatabaseParameters::defaultParameters(const QString databaseType)
{
    DatabaseParameters parameters;

    // only the database name is needed
    DatabaseConfigElement config = DatabaseConfigElement::element(databaseType);
    parameters.databaseType      = databaseType;
    parameters.databaseName      = config.databaseName;
    parameters.hostName          = config.hostName;
    parameters.userName          = config.userName;
    parameters.password          = config.password;
    parameters.port              = config.port.toInt();
    const QString miscDir        = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("digikam/db_misc");
    QString connectOptions       = config.connectOptions;
    connectOptions.replace(QLatin1String("$$DBMISCPATH$$"), miscDir);

    parameters.connectOptions   = connectOptions;

    qCDebug(DIGIKAM_GENERAL_LOG) << "ConnectOptions "<< parameters.connectOptions;
    return parameters;
}

DatabaseParameters DatabaseParameters::thumbnailParameters() const
{
    DatabaseParameters params = *this;
    params.databaseName       = databaseNameThumbnails;
    return params;
}

DatabaseParameters DatabaseParameters::parametersForSQLite(const QString& databaseFile)
{
    // only the database name is needed
    DatabaseParameters params(QLatin1String("QSQLITE"), databaseFile);
    params.setDatabasePath(databaseFile);
    params.setThumbsDatabasePath(params.getDatabaseNameOrDir());
    return params;
}

DatabaseParameters DatabaseParameters::parametersForSQLiteDefaultFile(const QString& directory)
{
    return parametersForSQLite(QDir::cleanPath(directory + QDir::separator() + QLatin1String(digikam4db)));
}

void DatabaseParameters::insertInUrl(QUrl& url) const
{
    removeFromUrl(url);

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("databaseType"), databaseType);
    q.addQueryItem(QLatin1String("databaseName"), databaseName);

    if (!connectOptions.isNull())
    {
        q.addQueryItem(QLatin1String("connectOptions"), connectOptions);
    }

    if (!hostName.isNull())
    {
        q.addQueryItem(QLatin1String("hostName"), hostName);
    }

    if (port != -1)
    {
        q.addQueryItem(QLatin1String("port"), QString::number(port));
    }

    if (internalServer)
    {
        q.addQueryItem(QLatin1String("internalServer"), QLatin1String("true"));
    }

    if (!userName.isNull())
    {
        q.addQueryItem(QLatin1String("userName"), userName);
    }

    if (!password.isNull())
    {
        q.addQueryItem(QLatin1String("password"), password);
    }

    url.setQuery(q);
}

void DatabaseParameters::removeFromUrl(QUrl& url)
{
    QUrlQuery q(url);
    q.removeQueryItem(QLatin1String("databaseType"));
    q.removeQueryItem(QLatin1String("databaseName"));
    q.removeQueryItem(QLatin1String("connectOptions"));
    q.removeQueryItem(QLatin1String("hostName"));
    q.removeQueryItem(QLatin1String("port"));
    q.removeQueryItem(QLatin1String("internalServer"));
    q.removeQueryItem(QLatin1String("userName"));
    q.removeQueryItem(QLatin1String("password"));
    url.setQuery(q);
}

QDebug operator<<(QDebug dbg, const DatabaseParameters& p)
{
    dbg.nospace() << "DatabaseParameters: [ Type " << p.databaseType           << ", ";
    dbg.nospace() << "Name "                       << p.databaseName           << " ";
    dbg.nospace() << "(Thumbnails Name "           << p.databaseNameThumbnails << "); ";

    if (!p.connectOptions.isEmpty())
    {
        dbg.nospace() << "ConnectOptions: "
                      << p.connectOptions << ", ";
    }

    if (!p.hostName.isEmpty())
    {
        dbg.nospace() << "Host Name and Port: "
                      << p.hostName << " " << p.port << "; ";
    }

    if (p.internalServer)
    {
        dbg.nospace() << "Using an Internal Server; ";
    }

    if (!p.userName.isEmpty())
    {
        dbg.nospace() << "Username and Password: "
                      << p.userName << ", " << p.password;
    }

    dbg.nospace() << "] ";

    return dbg.space();
}

}  // namespace Digikam
