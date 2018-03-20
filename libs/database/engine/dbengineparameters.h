/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Database Engine storage container for connection parameters.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

#ifndef DATABASE_ENGINE_PARAMETERS_H
#define DATABASE_ENGINE_PARAMETERS_H

// Qt includes

#include <QString>
#include <QtGlobal>
#include <QUrl>

// KDE includes

#include <ksharedconfig.h>

// Local includes

#include "digikam_export.h"
#include "dbengineconfig.h"

namespace Digikam
{

/**
 * This class encapsulates all parameters needed to establish
 * a connection to a database (inspired by the API of Qt::Sql).
 * The values can be read from and written to a QUrl.
 */
class DIGIKAM_EXPORT DbEngineParameters
{

public:

    DbEngineParameters(const QString& _type,
                       const QString& _databaseNameCore,
                       const QString& _connectOptions = QString(),
                       const QString& _hostName = QString(),
                       int            _port = -1,
                       bool           _internalServer = false,
                       const QString& _userName = QString(),
                       const QString& _password = QString(),
                       const QString& _databaseNameThumbnails = QString(),
                       const QString& _databaseNameFace = QString(),
                       const QString& _databaseNameSimilarity = QString(),
                       const QString& _internalServerDBPath = QString(),
                       const QString& _internalServerMysqlServCmd = QString(),
                       const QString& _internalServerMysqlInitCmd = QString()
                      );

    DbEngineParameters();

    /**
     * QUrl helpers.
     */
    explicit DbEngineParameters(const QUrl& url);
    void insertInUrl(QUrl& url) const;
    static void removeFromUrl(QUrl& url);

    bool operator==(const DbEngineParameters& other) const;
    bool operator!=(const DbEngineParameters& other) const;

    /**
     * Performs basic checks that the parameters are not empty and have the information
     * required for the databaseType.
     */
    bool    isValid()            const;

    bool    isSQLite()           const;
    bool    isMySQL()            const;
    QString SQLiteDatabaseFile() const;

    /**
     *  Returns the databaseType designating the said database.
     *  If you have a DbEngineParameters object already, you can use isSQLite() as well.
     *  These strings are identical to the driver identifiers in the Qt SQL module.
     */
    static QString SQLiteDatabaseType();
    static QString MySQLDatabaseType();

    /**
     * Creates a unique hash of the values stored in this object.
     */
    QByteArray hash() const;

    /**
     * Return a set of default parameters for the given type. For Mysql, it return internal server configuration.
     */
    static DbEngineParameters defaultParameters(const QString& databaseType);

    static DbEngineParameters parametersFromConfig(KSharedConfig::Ptr config = KSharedConfig::openConfig(),
                                                   const QString& configGroup = QString());
    /**
     * Read and write parameters from config. You can specify the group,
     * or use the default value.
     */
    void readFromConfig(KSharedConfig::Ptr config = KSharedConfig::openConfig(), const QString& configGroup = QString());
    void writeToConfig(KSharedConfig::Ptr config = KSharedConfig::openConfig(), const QString& configGroup = QString()) const;

    /**
     * NOTE: In case of SQLite, the database name typically is a file.
     * For non-SQLite, this simply handle the database name.
     */
    QString getCoreDatabaseNameOrDir()   const;
    QString getThumbsDatabaseNameOrDir() const;
    QString getFaceDatabaseNameOrDir()   const;
    QString getSimilarityDatabaseNameOrDir()   const;

    /// Use these methods if you set a file or a folder.
    void setCoreDatabasePath(const QString& folderOrFileOrName);
    void setThumbsDatabasePath(const QString& folderOrFileOrName);
    void setFaceDatabasePath(const QString& folderOrFileOrName);
    void setSimilarityDatabasePath(const QString& folderOrFileOrName);

    static QString coreDatabaseFileSQLite(const QString& folderOrFile);
    static QString thumbnailDatabaseFileSQLite(const QString& folderOrFile);
    static QString faceDatabaseFileSQLite(const QString& folderOrFile);
    static QString similarityDatabaseFileSQLite(const QString& folderOrFile);

    static QString coreDatabaseDirectorySQLite(const QString& path);
    static QString thumbnailDatabaseDirectorySQLite(const QString& path);
    static QString faceDatabaseDirectorySQLite(const QString& path);
    static QString similarityDatabaseDirectorySQLite(const QString& path);

    /**
     * For Mysql internal server: manage the database path to store database files.
     */
    void    setInternalServerPath(const QString& path);
    QString internalServerPath() const;

    /**
     * Replaces databaseName with databaseNameThumbnails.
     */
    DbEngineParameters thumbnailParameters() const;

    /**
     * Replaces databaseName with databaseNameFace.
     */
    DbEngineParameters faceParameters() const;

    /**
     * Replaces databaseName with databaseNameFace.
     */
    DbEngineParameters similarityParameters() const;

    void legacyAndDefaultChecks(const QString& suggestedPath = QString(), KSharedConfig::Ptr config = KSharedConfig::openConfig());
    void removeLegacyConfig(KSharedConfig::Ptr config);

    /**
     * Convenience methods to create a DbEngineParameters object for an
     * SQLITE database specified by the local file path.
     */
    static DbEngineParameters parametersForSQLite(const QString& databaseFile);
    static DbEngineParameters parametersForSQLiteDefaultFile(const QString& directory);

    /**
     * Return the hidden path from home directory to store private
     * data used by internal Mysql server.
     */
    static QString internalServerPrivatePath();
    
    /**
     * Return the default Mysql server command name (Internal server only).
     */
    static QString defaultMysqlServerCmd();

    /**
     * Return the default Mysql initialization command name (Internal server only).
     */
    static QString defaultMysqlInitCmd();
    
public:

    QString databaseType;
    QString databaseNameCore;
    QString connectOptions;
    QString hostName;
    int     port;
    bool    internalServer;
    QString userName;
    QString password;

    QString databaseNameThumbnails;
    QString databaseNameFace;
    QString databaseNameSimilarity;
    QString internalServerDBPath;

    /// Settings stored in config file and used only with internal server at runtime to start server instance or init database tables.
    QString internalServerMysqlServCmd;
    QString internalServerMysqlInitCmd;
};

DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const DbEngineParameters& t);

}  // namespace Digikam

#endif // DATABASE_ENGINE_PARAMETERS_H
