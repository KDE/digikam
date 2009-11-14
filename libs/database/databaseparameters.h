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
#include <QXmlStreamReader>
#include <QMap>
#include <QDomElement>

// KDE includes

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
    bool operator==(const DatabaseParameters& other) const;
    bool operator!=(const DatabaseParameters& other) const;

    /** Performs basic checks that the parameters are not empty and have the information
     *  required for the databaseType.
     */
    bool isValid() const;

    bool isSQLite() const;
    QString SQLiteDatabaseFile() const;

    /**
     * Creates a unique hash of the values stored in this object.
     */
    QByteArray hash() const;

    static DatabaseParameters parametersFromConfig(const QString &databaseType, const QString &databaseName,
            const QString &databaseHostName, int databasePort,
            const QString &databaseUserName, const QString &databaseUserPassword,
            const QString &databaseConnectOptions);

    /**
     * Convenience method to create a DatabaseParameters object for an
     * SQLITE 3 database specified by the local file path.
     */
    static DatabaseParameters parametersForSQLite(const QString& databaseFile);
    static DatabaseParameters parametersForSQLiteDefaultFile(const QString& directory);

    static void removeFromUrl(KUrl& url);

	void readConfig();
        QString m_DefaultDatabase;
        QMap<QString, databaseconfigelement> m_DatabaseConfigs;
protected:
        databaseconfigelement readDatabase(QDomElement& databaseElement);
        void readDBActions(QDomElement& sqlStatementElements, databaseconfigelement& configElement);
        void readStatements(QDomElement& sqlStatementElements, databaseconfigelement& configElement);

};

}  // namespace Digikam

#endif // DATABASEPARAMETERS_H
