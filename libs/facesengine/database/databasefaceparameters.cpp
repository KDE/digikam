/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for face database connection parameters.
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

#include "databasefaceparameters.h"

namespace FacesEngine
{

namespace
{
/*
static const char* configGroupDatabase = "Database Settings";
static const char* configDatabaseType  = "Database Type";
static const char* configDatabaseName  = "Database Name";
*/
}

DatabaseFaceParameters::DatabaseFaceParameters()
{
}

DatabaseFaceParameters::DatabaseFaceParameters(const QString& type, const QString& databaseName)
    : databaseType(type), databaseName(databaseName)
{
}

bool DatabaseFaceParameters::operator==(const DatabaseFaceParameters& other) const
{
    return (databaseType == other.databaseType &&
            databaseName == other.databaseName);
}

bool DatabaseFaceParameters::operator!=(const DatabaseFaceParameters& other) const
{
    return !operator==(other);
}

bool DatabaseFaceParameters::isValid() const
{
    if (isSQLite())
    {
        return !databaseName.isEmpty();
    }

    return false;
}

bool DatabaseFaceParameters::isSQLite() const
{
    return databaseType == QString::fromLatin1("QSQLITE");
}

bool DatabaseFaceParameters::isMySQL() const
{
    return databaseType == QString::fromLatin1("QMYSQL");
}

QString DatabaseFaceParameters::SQLiteDatabaseType()
{
    return QString::fromLatin1("QSQLITE");
}

QString DatabaseFaceParameters::MySQLDatabaseType()
{
    return QString::fromLatin1("QMYSQL");
}

QString DatabaseFaceParameters::SQLiteDatabaseFile() const
{
    if (isSQLite())
    {
        return databaseName;
    }

    return QString();
}

/*
DatabaseFaceParameters DatabaseFaceParameters::parametersFromConfig(KSharedConfig::Ptr config, const QString& configGroup)
{
    DatabaseFaceParameters parameters;
    parameters.readFromConfig(config, configGroup);
    return parameters;
}

void DatabaseFaceParameters::readFromConfig(KSharedConfig::Ptr config, const QString& configGroup)
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
    databaseName             = group.readEntry(configDatabaseName, QString());
}

void DatabaseFaceParameters::writeToConfig(KSharedConfig::Ptr config, const QString& configGroup) const
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

    group.writeEntry(configDatabaseType, databaseType);
    group.writeEntry(configDatabaseName, databaseName);

*/

DatabaseFaceParameters DatabaseFaceParameters::parametersForSQLite(const QString& databaseFile)
{
    // only the database name is needed
    return DatabaseFaceParameters(QString::fromLatin1("QSQLITE"), databaseFile);
}

} // namespace FacesEngine
