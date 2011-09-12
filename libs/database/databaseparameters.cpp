/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for database connection parameters.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Holger Foerster <hamsi2k at freenet dot de>
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

#include "config-digikam.h"
#include "databaseparameters.h"

// Qt includes

#include <QDir>
#include <QFile>

// KDE includes

#include <kcodecs.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// Local includes

#include "config-digikam.h"

namespace Digikam
{

static const char* configGroupDatabase = "Database Settings";
static const char* configInternalDatabaseServer = "Internal Database Server";

static const char* configImgDatabaseType = "Images Type";
static const char* configImgDatabaseName = "Images Name";
static const char* configImgDatabaseHostName = "Images Hostname";
static const char* configImgDatabasePort = "Images Port";
static const char* configImgDatabaseUsername = "Images Username";
static const char* configImgDatabasePassword = "Images Password";
static const char* configImgDatabaseConnectOptions = "Images Connectoptions";

static const char* configTmbDatabaseType = "Thumbnails Type";
static const char* configTmbDatabaseName = "Thumbnails Name";
static const char* configTmbDatabaseHostName = "Thumbnails Hostname";
static const char* configTmbDatabasePort = "Thumbnails Port";
static const char* configTmbDatabaseUsername = "Thumbnails Username";
static const char* configTmbDatabasePassword = "Thumbnails Password";
static const char* configTmbDatabaseConnectOptions = "Thumbnails Connectoptions";

// legacy
static const char* configDatabaseType = "Database Type";
static const char* configDatabaseName = "Database Name";
static const char* configDatabaseNameThumbnails = "Database Name Thumbnails";
static const char* configDatabaseHostName = "Database Hostname";
static const char* configDatabasePort = "Database Port";
static const char* configDatabaseUsername = "Database Username";
static const char* configDatabasePassword = "Database Password";
static const char* configDatabaseConnectOptions = "Database Connectoptions";
static const char* configDatabaseFilePathEntry = "Database File Path";
static const char* configAlbumPathEntry = "Album Path";

DatabaseParameters::DatabaseParameters()
    : imgPort(-1), tmbPort(-1)
{
}

DatabaseParameters::DatabaseParameters(const QString& imgType,
                                       const QString& tmbType,
                                       const QString& imgDatabaseName,
                                       const QString& tmbDatabaseName,
                                       const QString& imgConnectOptions,
                                       const QString& tmbConnectOptions,
                                       const QString& imgHostName,
                                       const QString& tmbHostName,
                                                   int imgPort,
                                                   int tmbPort,
                                       const QString& imgUserName,
                                       const QString& tmbUserName,
                                       const QString& imgPassword,
                                       const QString& tmbPassword,
                                       bool internalServer)
    : imgDatabaseType(imgType),
      tmbDatabaseType(tmbType),
      imgDatabaseName(imgDatabaseName),
      tmbDatabaseName(tmbDatabaseName),
      imgConnectOptions(imgConnectOptions),
      tmbConnectOptions(tmbConnectOptions),
      imgHostName(imgHostName),
      tmbHostName(tmbHostName),
      imgPort(imgPort),
      tmbPort(tmbPort),
      imgUserName(imgUserName),
      tmbUserName(tmbUserName),
      imgPassword(imgPassword),
      tmbPassword(tmbPassword),
      internalServer(internalServer)
{
}

DatabaseParameters::DatabaseParameters(const KUrl& url)
    :imgPort(-1), tmbPort(-1), internalServer(false)
{

#ifdef HAVE_INTERNALMYSQL
    QString queryServer = url.queryItem("internalServer");

    if (!queryServer.isNull())
    {
        internalServer = (queryServer == "true");
    }
#else
    internalServer = false;
#endif // HAVE_INTERNALMYSQL

    imgDatabaseType   = url.queryItem("imgDatabaseType");
    imgDatabaseName   = url.queryItem("imgDatabaseName");
    imgConnectOptions = url.queryItem("imgConnectOptions");
    imgHostName       = url.queryItem("imgHostName");
    QString queryPort = url.queryItem("imgPort");
    imgUserName       = url.queryItem("imgUserName");
    imgPassword       = url.queryItem("imgPassword");

    if (!queryPort.isNull())
    {
        imgPort = queryPort.toInt();
    }

    tmbDatabaseType   = url.queryItem("tmbDatabaseType");
    tmbDatabaseName   = url.queryItem("tmbDatabaseName");
    tmbConnectOptions = url.queryItem("tmbConnectOptions");
    tmbHostName       = url.queryItem("tmbHostName");
    queryPort         = url.queryItem("tmbPort");
    tmbUserName       = url.queryItem("tmbUserName");
    tmbPassword       = url.queryItem("tmbPassword");

    if (!queryPort.isNull())
    {
        tmbPort = queryPort.toInt();
    }

//     // legacy
//     if (imgDatabaseType.isNull())
//     {
//         kDebug(50003) << "Legacy configuration found, adapting ";
//         imgDatabaseType   = url.queryItem("databaseType");
//         imgDatabaseName   = url.queryItem("databaseName");
//         imgConnectOptions = url.queryItem("connectOptions");
//         imgHostName       = url.queryItem("hostName");
//         QString queryPort = url.queryItem("port");
//         imgUserName       = url.queryItem("userName");
//         imgPassword       = url.queryItem("password");
// 
//         tmbDatabaseType   = url.queryItem("databaseType");
//         tmbDatabaseName   = url.queryItem("databaseNameThumbnails");
//         tmbConnectOptions = url.queryItem("connectOptions");
//         tmbHostName       = url.queryItem("hostName");
//         QString queryPort = url.queryItem("port");
//         tmbUserName       = url.queryItem("userName");
//         tmbPassword       = url.queryItem("password");
// 
//         if (!queryPort.isNull())
//         {
//             imgPort = queryPort.toInt();
//             tmbPort = queryPort.toInt();
//         }
//     }
}

bool DatabaseParameters::operator==(const DatabaseParameters& other) const
{
    return internalServer    == other.internalServer &&
           imgDatabaseType   == other.imgDatabaseType &&
           imgDatabaseName   == other.imgDatabaseName &&
           imgConnectOptions == other.imgConnectOptions &&
           imgHostName       == other.imgHostName &&
           imgPort           == other.imgPort &&
           imgUserName       == other.imgUserName &&
           imgPassword       == other.imgPassword &&
           tmbDatabaseType   == other.tmbDatabaseType &&
           tmbDatabaseName   == other.tmbDatabaseName &&
           tmbConnectOptions == other.tmbConnectOptions &&
           tmbHostName       == other.tmbHostName &&
           tmbPort           == other.tmbPort &&
           tmbUserName       == other.tmbUserName &&
           tmbPassword       == other.tmbPassword;
}

bool DatabaseParameters::operator!=(const DatabaseParameters& other) const
{
    return !operator==(other);
}

bool DatabaseParameters::isValid() const
{
    if (isImgSQLite())
    {
        return !imgDatabaseName.isEmpty() &&
               !tmbDatabaseName.isEmpty();
    }

    return false;
}

bool DatabaseParameters::isImgSQLite() const
{
    return imgDatabaseType == "QSQLITE";
}

bool DatabaseParameters::isImgMySQL() const
{
    return imgDatabaseType == "QMYSQL";
}

bool DatabaseParameters::isTmbSQLite() const
{
    return tmbDatabaseType == "QSQLITE";
}

bool DatabaseParameters::isTmbMySQL() const
{
    return tmbDatabaseType == "QMYSQL";
}

QString DatabaseParameters::SQLiteDatabaseType()
{
    return "QSQLITE";
}

QString DatabaseParameters::MySQLDatabaseType()
{
    return "QMYSQL";
}

QString DatabaseParameters::imgSQLiteDatabaseFile() const
{
    if (isImgSQLite())
    {
        return imgDatabaseName;
    }

    return QString();
}

QString DatabaseParameters::tmbSQLiteDatabaseFile() const
{
    if (isTmbSQLite())
    {
        return tmbDatabaseName;
    }

    return QString();
}

QByteArray DatabaseParameters::hash() const
{
    KMD5 md5;
    md5.update(imgDatabaseType.toUtf8());
    md5.update(imgDatabaseName.toUtf8());
    md5.update(imgConnectOptions.toUtf8());
    md5.update(imgHostName.toUtf8());
    md5.update((const char*)&imgPort, sizeof(int));
    md5.update(imgUserName.toUtf8());
    md5.update(imgPassword.toUtf8());

    md5.update(tmbDatabaseType.toUtf8());
    md5.update(tmbDatabaseName.toUtf8());
    md5.update(tmbConnectOptions.toUtf8());
    md5.update(tmbHostName.toUtf8());
    md5.update((const char*)&tmbPort, sizeof(int));
    md5.update(tmbUserName.toUtf8());
    md5.update(tmbPassword.toUtf8());

    return md5.hexDigest();
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

#ifdef HAVE_INTERNALMYSQL
    internalServer           = group.readEntry(configInternalDatabaseServer, false);
#else
    internalServer           = false;
#endif // HAVE_INTERNALMYSQL

    imgDatabaseType             = group.readEntry(configImgDatabaseType, QString());
    imgDatabaseName             = group.readEntry(configImgDatabaseName, QString());
    imgHostName                 = group.readEntry(configImgDatabaseHostName, QString());
    imgPort                     = group.readEntry(configImgDatabasePort, -1);
    imgUserName                 = group.readEntry(configImgDatabaseUsername, QString());
    imgPassword                 = group.readEntry(configImgDatabasePassword, QString());
    imgConnectOptions           = group.readEntry(configImgDatabaseConnectOptions, QString());

    tmbDatabaseType             = group.readEntry(configTmbDatabaseType, QString());
    tmbDatabaseName             = group.readEntry(configTmbDatabaseName, QString());
    tmbHostName                 = group.readEntry(configTmbDatabaseHostName, QString());
    tmbPort                     = group.readEntry(configTmbDatabasePort, -1);
    tmbUserName                 = group.readEntry(configTmbDatabaseUsername, QString());
    tmbPassword                 = group.readEntry(configTmbDatabasePassword, QString());
    tmbConnectOptions           = group.readEntry(configTmbDatabaseConnectOptions, QString());

//     if (imgDatabaseType.isNull())
//     {
//         kDebug(50003) << "Legacy configuration found, adapting ";
// 
//         imgDatabaseType         = group.readEntry(configDatabaseType, QString());
//         imgDatabaseName         = group.readEntry(configDatabaseName, QString());
//         imgHostName             = group.readEntry(configDatabaseHostName, QString());
//         imgPort                 = group.readEntry(configDatabasePort, -1);
//         imgUserName             = group.readEntry(configDatabaseUsername, QString());
//         imgPassword             = group.readEntry(configDatabasePassword, QString());
//         imgConnectOptions       = group.readEntry(configDatabaseConnectOptions, QString());
// 
//         tmbDatabaseType         = group.readEntry(configDatabaseType, QString());
//         tmbDatabaseName         = group.readEntry(configDatabaseNameThumbnails, QString());
//         tmbHostName             = group.readEntry(configDatabaseHostName, QString());
//         tmbPort                 = group.readEntry(configDatabasePort, -1);
//         tmbUserName             = group.readEntry(configDatabaseUsername, QString());
//         tmbPassword             = group.readEntry(configDatabasePassword, QString());
//         tmbConnectOptions       = group.readEntry(configDatabaseConnectOptions, QString());
//     }

    if (isImgSQLite() && !imgDatabaseName.isNull())
    {
        QString orgName = imgDatabaseName;
        setImgDatabasePath(orgName);
    }
    if (isTmbSQLite() && !tmbDatabaseName.isNull())
    {
        QString orgName = tmbDatabaseName;
        setTmbDatabasePath(orgName);
    }
}

void DatabaseParameters::setImgDatabasePath(const QString& folderOrFileOrName)
{
    if (isImgSQLite())
    {
        imgDatabaseName = databaseFileSQLite(folderOrFileOrName, QString(DIGIKAM4DB));
    }
    else
    {
        imgDatabaseName = folderOrFileOrName;
    }
}

void DatabaseParameters::setTmbDatabasePath(const QString& folderOrFileOrName)
{
    if (isImgSQLite())
    {
        tmbDatabaseName = databaseFileSQLite(folderOrFileOrName, QString(THUMBNAILS_DIGIKAMDB));
    }
    else
    {
        tmbDatabaseName = folderOrFileOrName;
    }
}

QString DatabaseParameters::databaseFileSQLite(const QString& folderOrFile, const QString& fileName)
{
    QFileInfo fileInfo(folderOrFile);

    if (fileInfo.isDir())
    {
        return QDir::cleanPath(fileInfo.filePath() + QDir::separator() + fileName);
    }

    return QDir::cleanPath(folderOrFile);
}

void DatabaseParameters::legacyAndDefaultChecks(const QString& suggestedPath, KSharedConfig::Ptr config)
{
    // Additional semantic checks for the database section.
    // If the internal server should be started, then the connection options must be reset
    if (imgDatabaseType == "QMYSQL" && internalServer)
    {
        const QString miscDir = KStandardDirs::locateLocal("data", "digikam/db_misc");
        imgDatabaseType= "QMYSQL";
        imgDatabaseName = "digikam";
        imgHostName = QString();
        imgPort = -1;
        imgUserName = "root";
        imgPassword = QString();
        imgConnectOptions = QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir);
        tmbDatabaseType=    "QMYSQL";
        tmbDatabaseName =   "digikam";
        tmbHostName =       QString();
        tmbPort =           -1;
        tmbUserName =       "root";
        tmbPassword =       QString();
        tmbConnectOptions = QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir);
    }

    if (imgDatabaseType.isEmpty())
    {
        // Empty 2.1 config: migration from older versions
        KConfigGroup group = config->group(configGroupDatabase);

        kDebug(50003) << "Trying to read legacy configuration";

        if (group.hasKey(configDatabaseType))
        {
            kDebug(50003) << configDatabaseType << " Found.";
            imgDatabaseType         = group.readEntry(configDatabaseType, QString());
            imgDatabaseName         = group.readEntry(configDatabaseName, QString());
            if (imgDatabaseType == QString::fromLatin1("QSQLITE")) {
                imgDatabaseName = databaseFileSQLite(imgDatabaseName, QString::fromLatin1(DIGIKAM4DB));
            }
            imgHostName             = group.readEntry(configDatabaseHostName, QString());
            imgPort                 = group.readEntry(configDatabasePort, -1);
            imgUserName             = group.readEntry(configDatabaseUsername, QString());
            imgPassword             = group.readEntry(configDatabasePassword, QString());
            imgConnectOptions       = group.readEntry(configDatabaseConnectOptions, QString());

            tmbDatabaseType         = group.readEntry(configDatabaseType, QString());
            tmbDatabaseName         = group.readEntry(configDatabaseNameThumbnails, QString());
            if (tmbDatabaseType == QString::fromLatin1("QSQLITE")) {
                tmbDatabaseName = databaseFileSQLite(tmbDatabaseName, QString::fromLatin1(THUMBNAILS_DIGIKAMDB));
            }
            tmbHostName             = group.readEntry(configDatabaseHostName, QString());
            tmbPort                 = group.readEntry(configDatabasePort, -1);
            tmbUserName             = group.readEntry(configDatabaseUsername, QString());
            tmbPassword             = group.readEntry(configDatabasePassword, QString());
            tmbConnectOptions       = group.readEntry(configDatabaseConnectOptions, QString());
        }
        else
        {
            kDebug(50003) << "Empty 1.3 config";
            // Empty 1.3 config
            group  = config->group("Album Settings");

            QString databaseFilePath;

            // 1.0 - 1.2 style database file path?
            if (group.hasKey(configDatabaseFilePathEntry))
            {
                databaseFilePath = group.readEntry(configDatabaseFilePathEntry, QString());
            }
            // <= 0.9 style album path entry?
            else if (group.hasKey(configAlbumPathEntry))
            {
                databaseFilePath = group.readEntry(configAlbumPathEntry, QString());
            }
            else if (!suggestedPath.isNull())
            {
                databaseFilePath = suggestedPath;
            }

            if (!databaseFilePath.isEmpty())
            {
                *this = parametersForSQLite(databaseFileSQLite(databaseFilePath, QString(DIGIKAM4DB)));
            }

            // Be aware that schema updating from  <= 0.9 requires reading the "Album Path", so dont remove it here
        }
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

    QString dbImgName = getImgDatabaseNameOrDir();
    QString dbTmbName = getTmbDatabaseNameOrDir();

    group.writeEntry(configImgDatabaseType, imgDatabaseType);
    group.writeEntry(configImgDatabaseName, dbImgName);
    group.writeEntry(configImgDatabaseHostName, imgHostName);
    group.writeEntry(configImgDatabasePort, imgPort);
    group.writeEntry(configImgDatabaseUsername, imgUserName);
    group.writeEntry(configImgDatabasePassword, imgPassword);
    group.writeEntry(configImgDatabaseConnectOptions, imgConnectOptions);

    group.writeEntry(configTmbDatabaseType, tmbDatabaseType);
    group.writeEntry(configTmbDatabaseName, dbTmbName);
    group.writeEntry(configTmbDatabaseHostName, tmbHostName);
    group.writeEntry(configTmbDatabasePort, tmbPort);
    group.writeEntry(configTmbDatabaseUsername, tmbUserName);
    group.writeEntry(configTmbDatabasePassword, tmbPassword);
    group.writeEntry(configTmbDatabaseConnectOptions, tmbConnectOptions);

    group.writeEntry(configInternalDatabaseServer, internalServer);

}

QString DatabaseParameters::getImgDatabaseNameOrDir() const
{
    if (isImgSQLite())
    {
        return imgDatabaseDirectorySQLite(imgDatabaseName);
    }

    return imgDatabaseName;
}

QString DatabaseParameters::getTmbDatabaseNameOrDir() const
{
    if (isTmbSQLite())
    {
        return tmbDatabaseDirectorySQLite(tmbDatabaseName);
    }

    return tmbDatabaseName;
}

QString DatabaseParameters::imgDatabaseDirectorySQLite(const QString& path)
{
    if (path.endsWith(DIGIKAM4DB))
    {
        QString chopped(path);
        chopped.chop(QString(DIGIKAM4DB).length());
        return chopped;
    }

    return path;
}

QString DatabaseParameters::tmbDatabaseDirectorySQLite(const QString& path)
{
    if (path.endsWith(THUMBNAILS_DIGIKAMDB))
    {
        QString chopped(path);
        chopped.chop(QString(THUMBNAILS_DIGIKAMDB).length());
        return chopped;
    }

    return path;
}

DatabaseParameters DatabaseParameters::defaultParameters(const QString databaseType, const bool isInternal)
{
    DatabaseParameters parameters;

    // only the database name is needed
    DatabaseConfigElement config = DatabaseConfigElement::element(databaseType);

    parameters.tmbDatabaseType = parameters.imgDatabaseType = databaseType;
    parameters.tmbDatabaseName = parameters.imgDatabaseName = config.databaseName;
    parameters.tmbHostName     = parameters.imgHostName     = config.hostName;
    parameters.tmbUserName     = parameters.imgUserName     = config.userName;
    parameters.tmbPassword     = parameters.imgPassword     = config.password;
    parameters.tmbPort         = parameters.imgPort         = config.port.toInt();

    if(isInternal) 
    {
        const QString miscDir      = KStandardDirs::locateLocal("data", "digikam/db_misc");
        QString connectOptions = config.connectOptions;
        connectOptions.replace(QString("$$DBMISCPATH$$"), miscDir);
        parameters.tmbConnectOptions = parameters.imgConnectOptions = connectOptions;
    }
    parameters.tmbConnectOptions = parameters.imgConnectOptions = QLatin1String("");

    kDebug(50003) << "ConnectOptions "<< parameters.imgConnectOptions;
    return parameters;
}

DatabaseParameters DatabaseParameters::thumbnailParameters() const
{
    DatabaseParameters parameters = *this;

    parameters.imgDatabaseType   = tmbDatabaseType;
    parameters.imgDatabaseName   = tmbDatabaseName;
    parameters.imgHostName       = tmbHostName;
    parameters.imgUserName       = tmbUserName;
    parameters.imgPassword       = tmbPassword;
    parameters.imgPort           = tmbPort;
    parameters.imgConnectOptions = tmbConnectOptions;

    return parameters;
}

DatabaseParameters DatabaseParameters::parametersForSQLite(const QString& databaseFile)
{
    // this function should only be used on first run, empty settings
    // only the database name is needed
    DatabaseParameters params("QSQLITE", "QSQLITE", databaseFile, databaseFile);
    params.setImgDatabasePath(databaseFile);
    params.setTmbDatabasePath(params.getTmbDatabaseNameOrDir());
    return params;
}

DatabaseParameters DatabaseParameters::parametersForSQLiteDefaultFile(const QString& directory)
{
    // this function should only be used on first run, empty settings
    return parametersForSQLite(QDir::cleanPath(directory + QDir::separator() + DIGIKAM4DB));
}

void DatabaseParameters::insertInUrl(KUrl& url) const
{
    removeFromUrl(url);

    if (internalServer)
    {
        url.addQueryItem("internalServer", "true");
    }

    url.addQueryItem("imgDatabaseType", imgDatabaseType);
    url.addQueryItem("imgDatabaseName", imgDatabaseName);

    if (!imgConnectOptions.isNull())
    {
        url.addQueryItem("imgConnectOptions", imgConnectOptions);
    }

    if (!imgHostName.isNull())
    {
        url.addQueryItem("imgHostName", imgHostName);
    }

    if (imgPort != -1)
    {
        url.addQueryItem("imgPort", QString::number(imgPort));
    }

    if (!imgUserName.isNull())
    {
        url.addQueryItem("imgUserName", imgUserName);
    }

    if (!imgPassword.isNull())
    {
        url.addQueryItem("imgPassword", imgPassword);
    }

    url.addQueryItem("tmbDatabaseType", tmbDatabaseType);
    url.addQueryItem("tmbDatabaseName", tmbDatabaseName);


    if (!tmbConnectOptions.isNull())
    {
        url.addQueryItem("tmbConnectOptions", tmbConnectOptions);
    }

    if (!tmbHostName.isNull())
    {
        url.addQueryItem("tmbHostName", tmbHostName);
    }

    if (tmbPort != -1)
    {
        url.addQueryItem("tmbPort", QString::number(tmbPort));
    }

    if (!tmbUserName.isNull())
    {
        url.addQueryItem("tmbUserName", tmbUserName);
    }

    if (!tmbPassword.isNull())
    {
        url.addQueryItem("tmbPassword", tmbPassword);
    }
}

void DatabaseParameters::removeFromUrl(KUrl& url)
{
    url.removeQueryItem("internalServer");
    url.removeQueryItem("imgDatabaseType");
    url.removeQueryItem("imgDatabaseName");
    url.removeQueryItem("imgConnectOptions");
    url.removeQueryItem("imgHostName");
    url.removeQueryItem("imgPort");
    url.removeQueryItem("imgUserName");
    url.removeQueryItem("imgPassword");
    url.removeQueryItem("tmbDatabaseType");
    url.removeQueryItem("tmbDatabaseName");
    url.removeQueryItem("tmbConnectOptions");
    url.removeQueryItem("tmbHostName");
    url.removeQueryItem("tmbPort");
    url.removeQueryItem("tmbUserName");
    url.removeQueryItem("tmbPassword");
}

QDebug operator<<(QDebug dbg, const DatabaseParameters& p)
{
    dbg.nospace() << "DatabaseParameters: [ Type "
                  << p.imgDatabaseType << ", ";
    dbg.nospace() << "Name "
                  << p.imgDatabaseName << " ";

    if (!p.imgConnectOptions.isEmpty())
        dbg.nospace() << "ConnectOptions: "
                      << p.imgConnectOptions << ", ";

    if (!p.imgHostName.isEmpty())
        dbg.nospace() << "Host Name and Port: "
                      << p.imgHostName << " " << p.imgPort << "; ";

    if (!p.imgUserName.isEmpty())
        dbg.nospace() << "Username and Password: "
                      << p.imgUserName << ", " << p.imgPassword;

    if (p.internalServer)
    {
        dbg.nospace() << "Using an Internal Server; ";
    }
    else
    {

    dbg.nospace() << "tmb.Type "
                  << p.tmbDatabaseType << ", ";
    dbg.nospace() << "tmb.Name "
                  << p.tmbDatabaseName << " ";

    if (!p.tmbConnectOptions.isEmpty())
        dbg.nospace() << "tmb.ConnectOptions: "
                      << p.tmbConnectOptions << ", ";

    if (!p.tmbHostName.isEmpty())
        dbg.nospace() << "tmb.Host Name and Port: "
                      << p.tmbHostName << " " << p.tmbPort << "; ";

    if (!p.tmbUserName.isEmpty())
        dbg.nospace() << "tmb.Username and Password: "
                      << p.tmbUserName << ", " << p.tmbPassword;
    }
    dbg.nospace() << "] ";

    return dbg.space();
}

}  // namespace Digikam
