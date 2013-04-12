/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for database connection parameters.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEPARAMETERS_H
#define DATABASEPARAMETERS_H

// Qt includes

#include <QString>
#include <QtGlobal>

// KDE includes

#include <kconfig.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kurl.h>

// Local includes

#include "digikam_export.h"
#include "databaseconfigelement.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseParameters
{

public:

    /**
      * This class encapsulates all parameters needed to establish
      * a connection to a database (inspired by the API of QT SQL of Qt4).
      * The values can be read from and written to a KUrl.
      */

    DatabaseParameters(const QString& type,
                       const QString& databaseName,
                       const QString& connectOptions = QString(),
                       const QString& hostName = QString(),
                       int   port = -1,
                       bool  internalServer = false,
                       const QString& userName = QString(),
                       const QString& password = QString(),
                       const QString& databaseNameThumbnails = QString());

    explicit DatabaseParameters(const KUrl& url);
    DatabaseParameters();

    void insertInUrl(KUrl& url) const;
    bool operator==(const DatabaseParameters& other) const;
    bool operator!=(const DatabaseParameters& other) const;

    /** Performs basic checks that the parameters are not empty and have the information
     *  required for the databaseType.
     */
    bool    isValid()            const;

    bool    isSQLite()           const;
    bool    isMySQL()            const;
    QString SQLiteDatabaseFile() const;

    /**
     *  Returns the databaseType designating the said database.
     *  If you have a DatabaseParameters object already, you can use isSQLite() as well.
     *  These strings are identical to the driver identifiers in the Qt SQL module.
     */
    static QString SQLiteDatabaseType();
    static QString MySQLDatabaseType();

    /**
     * Creates a unique hash of the values stored in this object.
     */
    QByteArray hash() const;

    /** Return a set of default parameters for the give type */
    static DatabaseParameters defaultParameters(const QString databaseType);

    static DatabaseParameters parametersFromConfig(KSharedConfig::Ptr config = KGlobal::config(),
            const QString& configGroup = QString());
    /**
     * Read and write parameters from config. You can specify the group,
     * or use the default value.
     */
    void readFromConfig(KSharedConfig::Ptr config = KGlobal::config(), const QString& configGroup = QString());
    void writeToConfig(KSharedConfig::Ptr config = KGlobal::config(), const QString& configGroup = QString()) const;

    /**
     * In case of SQLite, the databaseName typically is a file.
     * With getDatabasePath, you can get the folder.
     */
    QString getDatabaseNameOrDir() const;
    QString getThumbsDatabaseNameOrDir() const;

    /**
     * In case of SQLite, the databaseName typically is a file.
     * Use this method if you set a file or a folder.
     * For non-SQLite, this simply sets the database name.
     */
    void setDatabasePath(const QString& folderOrFileOrName);
    void setThumbsDatabasePath(const QString& folderOrFileOrName);

    static QString databaseFileSQLite(const QString& folderOrFile);
    static QString thumbnailDatabaseFileSQLite(const QString& folderOrFile);
    static QString databaseDirectorySQLite(const QString& path);
    static QString thumbnailDatabaseDirectorySQLite(const QString& path);

    /** Replaces databaseName with databaseNameThumbnails. */
    DatabaseParameters thumbnailParameters() const;

    void legacyAndDefaultChecks(const QString& suggestedPath = QString(), KSharedConfig::Ptr config = KGlobal::config());
    void removeLegacyConfig(KSharedConfig::Ptr config);

    /**
     * Convenience method to create a DatabaseParameters object for an
     * SQLITE 3 database specified by the local file path.
     */
    static DatabaseParameters parametersForSQLite(const QString& databaseFile);
    static DatabaseParameters parametersForSQLiteDefaultFile(const QString& directory);

    static void removeFromUrl(KUrl& url);

public:

    QString databaseType;
    QString databaseName;
    QString connectOptions;
    QString hostName;
    int     port;
    bool    internalServer;
    QString userName;
    QString password;

    QString databaseNameThumbnails;
};

DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const DatabaseParameters& t);

}  // namespace Digikam

#endif // DATABASEPARAMETERS_H
