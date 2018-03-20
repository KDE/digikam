/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-01
 * Description : Similarity DB schema update
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
 * Copyright (C)      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

#include "similaritydbschemaupdater.h"

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
#include "similaritydbaccess.h"
#include "similaritydb.h"

namespace Digikam
{

int SimilarityDbSchemaUpdater::schemaVersion()
{
    return 1;
}

// -------------------------------------------------------------------------------------

class SimilarityDbSchemaUpdater::Private
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

    bool                    setError;

    int                     currentVersion;
    int                     currentRequiredVersion;

    SimilarityDbAccess*     access;

    InitializationObserver* observer;
};

SimilarityDbSchemaUpdater::SimilarityDbSchemaUpdater(SimilarityDbAccess* const access)
    : d(new Private)
{
    d->access = access;
}

SimilarityDbSchemaUpdater::~SimilarityDbSchemaUpdater()
{
    delete d;
}

bool SimilarityDbSchemaUpdater::update()
{
    bool success = startUpdates();

    // even on failure, try to set current version - it may have incremented
    if (d->currentVersion)
    {
        d->access->db()->setSetting(QLatin1String("DBSimilarityVersion"),
                                    QString::number(d->currentVersion));
    }

    if (d->currentRequiredVersion)
    {
        d->access->db()->setSetting(QLatin1String("DBSimilarityVersionRequired"),
                                    QString::number(d->currentRequiredVersion));
    }

    return success;
}

void SimilarityDbSchemaUpdater::setObserver(InitializationObserver* const observer)
{
    d->observer = observer;
}

bool SimilarityDbSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = d->access->backend()->tables();

    if (tables.contains(QLatin1String("SimilaritySettings"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        QString version         = d->access->db()->getSetting(QLatin1String("DBSimilarityVersion"));
        QString versionRequired = d->access->db()->getSetting(QLatin1String("DBSimilarityVersionRequired"));

        qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: have a structure version "
                                          << version;

        // mini schema update
        if (version.isEmpty() && d->access->parameters().isSQLite())
        {
            version = d->access->db()->getSetting(QLatin1String("DBVersion"));
        }

        if (version.isEmpty() && d->access->parameters().isMySQL())
        {
            version         = d->access->db()->getLegacySetting(QLatin1String("DBSimilarityVersion"));
            versionRequired = d->access->db()->getLegacySetting(QLatin1String("DBSimilarityVersionRequired"));
        }

        // We absolutely require the DBSimilarityVersion setting
        if (version.isEmpty())
        {
            // Something is damaged. Give up.

            qCCritical(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: database version not available! Giving up schema upgrading.";

            QString errorMsg = i18n("The database is not valid: "
                                    "the \"DBSimilarityVersion\" setting does not exist. "
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
            // trying to open a database with a more advanced than this SimilarityDbSchemaUpdater supports
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
        qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: no database file available";

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


bool SimilarityDbSchemaUpdater::createDatabase()
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

bool SimilarityDbSchemaUpdater::createTables()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QLatin1String("CreateSimilarityDB")));
}

bool SimilarityDbSchemaUpdater::createIndices()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QLatin1String("CreateSimilarityDBIndices")));
}

bool SimilarityDbSchemaUpdater::createTriggers()
{
    return d->access->backend()->execDBAction(d->access->backend()->getDBAction(QLatin1String("CreateSimilarityDBTriggers")));
}

bool SimilarityDbSchemaUpdater::makeUpdates()
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

bool SimilarityDbSchemaUpdater::updateV1ToV2()
{
    // Do nothing for now.
    return true;
}

} // namespace Digikam
