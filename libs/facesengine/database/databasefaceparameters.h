/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for face database connection parameters.
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

#ifndef DATABASE_FACE_PARAMETERS_H
#define DATABASE_FACE_PARAMETERS_H

// Qt includes

#include <QString>

// Local includes

#include "databasefaceconfig.h"

namespace FacesEngine
{

class DatabaseFaceParameters
{

public:

    /**
     * This class encapsulates all parameters needed to establish
     * a connection to a database (inspired by the API of QT SQL of Qt4).
     * The values can be read from and written to a KUrl.
     */
    DatabaseFaceParameters(const QString& type, const QString& databaseFilePath);
    DatabaseFaceParameters();

    QString databaseType;
    QString databaseName;
    QString connectOptions;

    bool operator==(const DatabaseFaceParameters& other) const;
    bool operator!=(const DatabaseFaceParameters& other) const;

    /**
     * Performs basic checks that the parameters are not empty and have the information
     * required for the databaseType.
     */
    bool isValid() const;

    bool    isSQLite() const;
    bool    isMySQL() const;
    QString SQLiteDatabaseFile() const;

    /**
     * Returns the databaseType designating the said database.
     * If you have a DatabaseFaceParameters object already, you can use isSQLite() as well.
     * These strings are identical to the driver identifiers in the Qt SQL module.
     */
    static QString SQLiteDatabaseType();
    static QString MySQLDatabaseType();

    /**
     * Return a set of default parameters for the give type
     */
    static DatabaseFaceParameters defaultParameters(const QString databaseType);

    //static DatabaseFaceParameters parametersFromConfig(KSharedConfig::Ptr config = KSharedConfig::openConfig(), const QString& configGroup = QString());

    /**
     * Read and write parameters from config. You can specify the group,
     * or use the default value.
     */
    //void readFromConfig(KSharedConfig::Ptr config = KSharedConfig::openConfig(), const QString& configGroup = QString());
    //void writeToConfig(KSharedConfig::Ptr config = KSharedConfig::openConfig(), const QString& configGroup = QString()) const;

    /**
     * Convenience method to create a DatabaseFaceParameters object for an
     * SQLITE 3 database specified by the local file path.
     */
    static DatabaseFaceParameters parametersForSQLite(const QString& databaseFile);
};

} // namespace FacesEngine

#endif // DATABASE_FACE_PARAMETERS_H
