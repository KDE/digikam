/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-24
 * Description : Database privileges checker
 *
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

#include "databasechecker.h"

// Qt includes

#include <QSqlDatabase>
#include <QSqlError>

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "databasebackend.h"

namespace Digikam
{

DatabasePrivilegesChecker::DatabasePrivilegesChecker(const DatabaseParameters& parameters)
{
    m_parameters = parameters;
}

DatabasePrivilegesChecker::~DatabasePrivilegesChecker()
{
}

bool DatabasePrivilegesChecker::checkPrivileges(QStringList& insufficientRights)
{
    bool result = true;
    DatabaseLocking fromLocking;
    DatabaseBackend fromDBbackend(&fromLocking, "PrivilegesCheckDatabase");

    if (!fromDBbackend.open(m_parameters))
    {
        return false;
    }

    if (!checkPriv(fromDBbackend, "CheckPriv_CREATE_TABLE"))
    {
        insufficientRights.append("CREATE TABLE");
        result = false;
    }
    else if (!checkPriv(fromDBbackend, "CheckPriv_ALTER_TABLE"))
    {
        insufficientRights.append("ALTER TABLE");
        result = false;
    }
    else if (!checkPriv(fromDBbackend, "CheckPriv_CREATE_TRIGGER"))
    {
        insufficientRights.append("CREATE TRIGGER");
        result = false;
    }
    else if (!checkPriv(fromDBbackend, "CheckPriv_DROP_TRIGGER"))
    {
        insufficientRights.append("DROP TRIGGER");
        result = false;
    }
    else if (!checkPriv(fromDBbackend, "CheckPriv_DROP_TABLE"))
    {
        insufficientRights.append("DROP TABLE");
        result = false;
    }

    // Try to delete this table in any case
    checkPriv(fromDBbackend, "CheckPriv_Cleanup");

    return result;
}

bool DatabasePrivilegesChecker::checkPriv(DatabaseBackend& dbBackend, const QString& dbActionName)
{
    QMap<QString, QVariant> bindingMap;
    // now perform the copy action
    QList<QString> columnNames;
    DatabaseCoreBackend::QueryState queryStateResult = dbBackend.execDBAction(dbBackend.getDBAction(dbActionName),
            bindingMap);

    if (queryStateResult != DatabaseCoreBackend::NoErrors &&
        dbBackend.lastSQLError().isValid()                &&
        dbBackend.lastSQLError().number() != 0)
    {
        kDebug() << "Error while creating a trigger. Details: " << dbBackend.lastSQLError();
        return false;
    }

    return true;
}

}  // namespace Digikam
