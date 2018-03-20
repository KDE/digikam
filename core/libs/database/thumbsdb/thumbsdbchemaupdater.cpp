/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbsdbchemaupdater.h"

// Qt includes

#include <QFileInfo>
#include <QFile>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "collectionscannerobserver.h"
#include "dbenginebackend.h"
#include "coredbtransaction.h"
#include "thumbsdbaccess.h"
#include "thumbsdb.h"

namespace Digikam
{

int ThumbsDbSchemaUpdater::schemaVersion()
{
    return 3;
}

// -------------------------------------------------------------------------------------

class ThumbsDbSchemaUpdater::Private
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

public:

    bool                     setError;

    int                      currentVersion;
    int                      currentRequiredVersion;

    ThumbsDbAccess*          access;

    InitializationObserver*  observer;
};

ThumbsDbSchemaUpdater::ThumbsDbSchemaUpdater(ThumbsDbAccess* const access)
    : d(new Private)
{
    d->access = access;
}

ThumbsDbSchemaUpdater::~ThumbsDbSchemaUpdater()
{
    delete d;
}

bool ThumbsDbSchemaUpdater::update()
{
    bool success = startUpdates();

    // even on failure, try to set current version - it may have incremented
    if (d->currentVersion)
    {
        d->access->db()->setSetting(QLatin1String("DBThumbnailsVersion"),
                                    QString::number(d->currentVersion));
    }

    if (d->currentRequiredVersion)
    {
        d->access->db()->setSetting(QLatin1String("DBThumbnailsVersionRequired"),
                                    QString::number(d->currentRequiredVersion));
    }

    return success;
}

void ThumbsDbSchemaUpdater::setObserver(InitializationObserver* const observer)
{
    d->observer = observer;
}

bool ThumbsDbSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = d->access->backend()->tables();

    if (tables.contains(QLatin1String("Thumbnails"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        QString version         = d->access->db()->getSetting(QLatin1String("DBThumbnailsVersion"));
        QString versionRequired = d->access->db()->getSetting(QLatin1String("DBThumbnailsVersionRequired"));
        qCDebug(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: have a structure version " << version;

        // mini schema update
        if (version.isEmpty() && d->access->parameters().isSQLite())
        {
            version = d->access->db()->getSetting(QLatin1String("DBVersion"));
        }

        if (version.isEmpty() && d->access->parameters().isMySQL())
        {
            version         = d->access->db()->getLegacySetting(QLatin1String("DBThumbnailsVersion"));
            versionRequired = d->access->db()->getLegacySetting(QLatin1String("DBThumbnailsVersionRequired"));
        }

        // We absolutely require the DBThumbnailsVersion setting
        if (version.isEmpty())
        {
            // Something is damaged. Give up.

            qCCritical(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: database version not available! Giving up schema upgrading.";

            QString errorMsg = i18n("The database is not valid: "
                                    "the \"DBThumbnailsVersion\" setting does not exist. "
                                    "The current database schema version cannot be verified. "
                                    "Try to start with an empty database. ");
            d->access->setLastError(errorMsg);

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        // current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.
        d->currentVersion = version.toInt();

        if (d->currentVersion > schemaVersion())
        {
            // trying to open a database with a more advanced than this ThumbsDbSchemaUpdater supports
            if (!versionRequired.isEmpty() && versionRequired.toInt() <= schemaVersion())
            {
                // version required may be less than current version
                return true;
            }
            else
            {
                QString errorMsg = i18n("The database has been used with a more recent version of digiKam "
                                        "and has been updated to a database schema which cannot be used with this version. "
                                        "(This means this digiKam version is too old, or the database format is to recent) "
                                        "Please use the more recent version of digikam that you used before. ");
                d->access->setLastError(errorMsg);

                if (d->observer)
                {
                    d->observer->error(errorMsg);
                    d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
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
        qCDebug(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: no database file available";

        DbEngineParameters parameters = d->access->parameters();

        // No legacy handling: start with a fresh db
        if (!createDatabase())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n ") +
                               d->access->backend()->lastError();
            d->access->setLastError(errorMsg);

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        return true;
    }
}

bool ThumbsDbSchemaUpdater::makeUpdates()
{
    if (d->currentVersion < schemaVersion())
    {
        if (d->currentVersion == 1)
        {
            updateV1ToV2();
        }
        if (d->currentVersion <= 2)
        {
            updateV2ToV3();
        }
    }

    return true;
}

bool ThumbsDbSchemaUpdater::createDatabase()
{
    if ( createTables()  &&
         createIndices() &&
         createTriggers())
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

bool ThumbsDbSchemaUpdater::createTables()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QLatin1String("CreateThumbnailsDB")));
}

bool ThumbsDbSchemaUpdater::createIndices()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QLatin1String("CreateThumbnailsDBIndices")));
}

bool ThumbsDbSchemaUpdater::createTriggers()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QLatin1String("CreateThumbnailsDBTrigger")));
}

bool ThumbsDbSchemaUpdater::updateV1ToV2()
{
    if (!d->access->backend()->execDBAction(d->access->backend()->getDBAction(QLatin1String("UpdateThumbnailsDBSchemaFromV1ToV2"))))
    {
        qCDebug(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: schema upgrade from V1 to V2 failed!";
        return false;
    }

    d->currentVersion         = 2;
    d->currentRequiredVersion = 1;
    return true;
}

bool ThumbsDbSchemaUpdater::updateV2ToV3()
{
    if (!d->access->backend()->execDBAction(d->access->backend()->getDBAction(QLatin1String("UpdateThumbnailsDBSchemaFromV2ToV3"))))
    {
        qCDebug(DIGIKAM_THUMBSDB_LOG) << "Thumbs database: schema upgrade from V2 to V3 failed!";
        return false;
    }

    d->currentVersion         = 3;
    d->currentRequiredVersion = 1;
    return true;
}

}  // namespace Digikam
