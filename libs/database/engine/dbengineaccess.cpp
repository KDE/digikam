/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Core database access wrapper.
 *
 * Copyright (C) 2016 by Swati Lodha <swatilodha27 at gmail dot com>
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

#include "dbengineaccess.h"

// Qt includes

#include <QEventLoop>
#include <QMutex>
#include <QSqlDatabase>
#include <QUuid>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dbengineparameters.h"
#include "dbenginebackend.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

bool DbEngineAccess::checkReadyForUse(QString& error)
{
    QStringList drivers = QSqlDatabase::drivers();

    // Retrieving DB settings from config file

    DbEngineParameters internalServerParameters = DbEngineParameters::parametersFromConfig(KSharedConfig::openConfig());

    // Checking for QSQLITE driver

    if(internalServerParameters.SQLiteDatabaseType() == QLatin1String("QSQLITE"))
    {
        if (!drivers.contains(QLatin1String("QSQLITE")))
        {
            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: no Sqlite3 driver available.\n"
                                           "List of QSqlDatabase drivers: " << drivers;

            error = i18n("The driver \"SQLITE\" for Sqlite3 databases is not available.\n"
                         "digiKam depends on the drivers provided by the Qt::SQL module.");
            return false;
        }
    }

    // Checking for QMYSQL driver

    else if(internalServerParameters.MySQLDatabaseType() == QLatin1String("QMYSQL"))
    {
        if (!drivers.contains(QLatin1String("QMYSQL")))
        {
            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: no MySQL driver available.\n"
                                           "List of QSqlDatabase drivers: " << drivers;

            error = i18n("The driver \"MYSQL\" for MySQL databases is not available.\n"
                         "digiKam depends on the drivers provided by the Qt::SQL module.");
            return false;
        }
    }
    else
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Database could not be found";
        error = QLatin1String("No valid database type available.");
        return false;
    }

    return true;
}

} // namespace Digikam
