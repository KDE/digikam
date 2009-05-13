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

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DatabaseParameters
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
                       const QString& userName = QString(),
                       const QString& password = QString());

    DatabaseParameters(const KUrl& url);
    DatabaseParameters();

    QString databaseType;
    QString databaseName;
    QString connectOptions;
    QString hostName;
    int port;
    QString userName;
    QString password;

    void insertInUrl(KUrl& url) const;
    bool operator==(const DatabaseParameters& other);
    bool operator!=(const DatabaseParameters& other);

    bool isSQLite() const;
    QString SQLiteDatabaseFile() const;

    /**
     * Creates a unique hash of the values stored in this object.
     */
    QByteArray hash() const;

    /**
     * Convenience method to create a DatabaseParameters object for an
     * SQLITE 3 database specified by the local file path.
     */
    static DatabaseParameters parametersForSQLite(const QString& databaseFile);
    static DatabaseParameters parametersForSQLiteDefaultFile(const QString& directory);

    static void removeFromUrl(KUrl& url);
};

}  // namespace Digikam

#endif // DATABASEPARAMETERS_H
