/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "databasefaceschemaupdater.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "databasecorebackend.h"
#include "databasefaceaccess.h"
#include "databasefaceinitobserver.h"
#include "trainingdb.h"

namespace FacesEngine
{

int DatabaseFaceSchemaUpdater::schemaVersion()
{
    return 2;
}

// -------------------------------------------------------------------------------------

class DatabaseFaceSchemaUpdater::Private
{
public:

    Private()
        : setError(false),
          currentVersion(0),
          currentRequiredVersion(0),
          access(0),
          observer(0)
    {
    }

    bool                      setError;

    int                       currentVersion;
    int                       currentRequiredVersion;

    DatabaseFaceAccess*       access;

    DatabaseFaceInitObserver* observer;
};

DatabaseFaceSchemaUpdater::DatabaseFaceSchemaUpdater(DatabaseFaceAccess* const access)
    : d(new Private)
{
    d->access = access;
}

DatabaseFaceSchemaUpdater::~DatabaseFaceSchemaUpdater()
{
    delete d;
}


void DatabaseFaceSchemaUpdater::setObserver(DatabaseFaceInitObserver* const observer)
{
    d->observer = observer;
}

bool DatabaseFaceSchemaUpdater::update()
{
    bool success = startUpdates();

    // even on failure, try to set current version - it may have incremented
    if (d->currentVersion)
    {
        d->access->db()->setSetting(QString::fromLatin1("DBVersion"), QString::number(d->currentVersion));
    }

    if (d->currentRequiredVersion)
    {
        d->access->db()->setSetting(QString::fromLatin1("DBVersionRequired"), QString::number(d->currentRequiredVersion));
    }

    return success;
}

bool DatabaseFaceSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = d->access->backend()->tables();

    if (tables.contains(QString::fromLatin1("Settings"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        QString version         = d->access->db()->setting(QString::fromLatin1("DBVersion"));
        QString versionRequired = d->access->db()->setting(QString::fromLatin1("DBVersionRequired"));
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Have a database structure version " << version;

        // mini schema update
        if (version.isEmpty() && d->access->parameters().isSQLite())
        {
            version = d->access->db()->setting(QString::fromLatin1("DBVersion"));
        }

        // We absolutely require the DBVersion setting
        if (version.isEmpty())
        {
            // Something is damaged. Give up.
            qCWarning(DIGIKAM_FACESENGINE_LOG) << "DBVersion not available! Giving up schema upgrading.";

            QString errorMsg = i18n("The database is not valid: "
                                    "the \"DBVersion\" setting does not exist. "
                                    "The current database schema version cannot be verified. "
                                    "Try to start with an empty database. ");

            d->access->setLastError(errorMsg);

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(DatabaseFaceInitObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        // current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.
        d->currentVersion = version.toInt();

        if (d->currentVersion > schemaVersion())
        {
            // trying to open a database with a more advanced than this DatabaseFaceSchemaUpdater supports
            if (!versionRequired.isEmpty() && versionRequired.toInt() <= schemaVersion())
            {
                // version required may be less than current version
                return true;
            }
            else
            {
                QString errorMsg = i18n("The database has been used with a more recent version of digiKam "
                                        "and has been updated to a database schema which cannot be used with this version. "
                                        "(This means this digiKam version is too old, or the database format is to recent.) "
                                        "Please use the more recent version of digiKam that you used before.");

                d->access->setLastError(errorMsg);

                if (d->observer)
                {
                    d->observer->error(errorMsg);
                    d->observer->finishedSchemaUpdate(DatabaseFaceInitObserver::UpdateErrorMustAbort);
                }

                return false;
            }
        }
        else
        {
            return makeUpdates();
        }
    }
    else
    {
        //qCDebug(DIGIKAM_FACESENGINE_LOG) << "No database file available";
        DatabaseFaceParameters parameters = d->access->parameters();

        // No legacy handling: start with a fresh db
        if (!createDatabase())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n%1",
                                    d->access->backend()->lastError().text());
            d->access->setLastError(errorMsg);

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(DatabaseFaceInitObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        return true;
    }
}

bool DatabaseFaceSchemaUpdater::makeUpdates()
{
    if (d->currentVersion < schemaVersion())
    {
        if (d->currentVersion == 1)
        {
            updateV1ToV2();
        }
    }

    return true;
}


bool DatabaseFaceSchemaUpdater::createDatabase()
{
    if ( createTables() && createIndices() && createTriggers())
    {
        d->currentVersion         = schemaVersion();
        d->currentRequiredVersion = 1;
        return true;
    }
    else
    {
        return false;
    }
}

bool DatabaseFaceSchemaUpdater::createTables()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateDB"))) &&
           d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateDBOpenCVLBPH")));
}

bool DatabaseFaceSchemaUpdater::createIndices()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateIndices")));
}

bool DatabaseFaceSchemaUpdater::createTriggers()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QString::fromLatin1("CreateTriggers")));
}

bool DatabaseFaceSchemaUpdater::updateV1ToV2()
{
/*
    if (!d->access->backend()->execDBAction(d->access->backend()->getDBAction("UpdateDBSchemaFromV1ToV2")))
    {
        qError() << "Schema upgrade in DB from V1 to V2 failed!";
        return false;
    }
*/

    d->currentVersion         = 2;
    d->currentRequiredVersion = 1;
    return true;
}

} // namespace FacesEngine
