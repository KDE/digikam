/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for database connection parameters.
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <QDir>

// Local includes.

#include "databaseparameters.h"

namespace Digikam
{

DatabaseParameters::DatabaseParameters()
    : port(-1)
{
}

DatabaseParameters::DatabaseParameters(const QString &type,
                                       const QString &databaseName,
                                       const QString &connectOptions,
                                       const QString &hostName,
                                       int port,
                                       const QString &userName,
                                       const QString &password)
    : databaseType(type), databaseName(databaseName),
      connectOptions(connectOptions), hostName(hostName),
      port(port), userName(userName),
      password(password)
{
}

DatabaseParameters::DatabaseParameters(const KUrl &url)
    : port(-1)
{
    databaseType   = url.queryItem("databaseType");
    databaseName   = url.queryItem("databaseName");
    connectOptions = url.queryItem("connectOptions");
    hostName       = url.queryItem("hostName");
    QString queryPort = url.queryItem("port");
    if (!queryPort.isNull())
        port = queryPort.toInt();
    userName       = url.queryItem("userName");
    password       = url.queryItem("password");
}

bool DatabaseParameters::operator==(const DatabaseParameters &other)
{
    return databaseType   == other.databaseType &&
           databaseName   == other.databaseName &&
           connectOptions == other.connectOptions &&
           hostName       == other.hostName &&
           port           == other.port &&
           userName       == other.userName &&
           password       == other.password;
}

bool DatabaseParameters::operator!=(const DatabaseParameters &other)
{
    return !operator==(other);
}

bool DatabaseParameters::isSQLite() const
{
    return databaseType == "QSQLITE";
}

DatabaseParameters DatabaseParameters::parametersForSQLite(const QString &databaseFile)
{
    // only the database name is needed
    return DatabaseParameters("QSQLITE", databaseFile);
}

DatabaseParameters DatabaseParameters::parametersForSQLiteDefaultFile(const QString &directory)
{
    QString filePath = directory + '/' + "digikam4.db";
    filePath = QDir::cleanPath(filePath);
    return parametersForSQLite(filePath);
}

void DatabaseParameters::insertInUrl(KUrl &url) const
{
    removeFromUrl(url);

    url.addQueryItem("databaseType", databaseType);
    url.addQueryItem("databaseName", databaseName);
    if (!connectOptions.isNull())
        url.addQueryItem("connectOptions", connectOptions);
    if (!hostName.isNull())
        url.addQueryItem("hostName", hostName);
    if (port != -1)
        url.addQueryItem("port", QString::number(port));
    if (!userName.isNull())
        url.addQueryItem("userName", userName);
    if (!password.isNull())
        url.addQueryItem("password", password);
}

void DatabaseParameters::removeFromUrl(KUrl &url)
{
    url.removeQueryItem("databaseType");
    url.removeQueryItem("databaseName");
    url.removeQueryItem("connectOptions");
    url.removeQueryItem("hostName");
    url.removeQueryItem("port");
    url.removeQueryItem("userName");
    url.removeQueryItem("password");
}

}  // namespace Digikam
