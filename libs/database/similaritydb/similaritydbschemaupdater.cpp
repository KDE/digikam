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

#include "similaritydbchemaupdater.h"

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
#include "similaritydbaccess.h"
#include "similaritydb.h"

namespace Digikam
{

int SimilarityDbSchemaUpdater::schemaVersion()
{
    return 1;
}

// -------------------------------------------------------------------------------------

SimilarityDbSchemaUpdater::SimilarityDbSchemaUpdater(SimilarityDbAccess* access)
{
    m_access                 = access;
    m_currentVersion         = 0;
    m_currentRequiredVersion = 0;
    m_observer               = 0;
    m_setError               = false;
}

bool SimilarityDbSchemaUpdater::update()
{
    bool success = startUpdates();

    // even on failure, try to set current version - it may have incremented
    if (m_currentVersion)
    {
        m_access->db()->setSetting(QLatin1String("DBSimilarityVersion"), QString::number(m_currentVersion));
    }

    if (m_currentRequiredVersion)
    {
        m_access->db()->setSetting(QLatin1String("DBSimilarityVersionRequired"), QString::number(m_currentRequiredVersion));
    }

    return success;
}

void SimilarityDbSchemaUpdater::setObserver(InitializationObserver* observer)
{
    m_observer = observer;
}

bool SimilarityDbSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = m_access->backend()->tables();

    if (tables.contains(QLatin1String("Settings"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        QString version         = m_access->db()->getSetting(QLatin1String("DBSimilarityVersion"));
        QString versionRequired = m_access->db()->getSetting(QLatin1String("DBSimilarityVersionRequired"));
        qCDebug(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: have a structure version " << version;

        // mini schema update
        if (version.isEmpty() && m_access->parameters().isSQLite())
        {
            version = m_access->db()->getSetting(QLatin1String("DBVersion"));
        }
        if (version.isEmpty() && m_access->parameters().isMySQL())
        {
            version         = m_access->db()->getLegacySetting(QLatin1String("DBSimilarityVersion"));
            versionRequired = m_access->db()->getLegacySetting(QLatin1String("DBSimilarityVersionRequired"));
        }

        // We absolutely require the DBSimilarityVersion setting
        if (version.isEmpty())
        {
            // Something is damaged. Give up.

            qCCritical(DIGIKAM_SIMILARITYDB_LOG) << "Similarity database: database version not available! Giving up schema upgrading.";

            QString errorMsg = i18n(
                                   "The database is not valid: "
                                   "the \"DBSimilarityVersion\" setting does not exist. "
                                   "The current database schema version cannot be verified. "
                                   "Try to start with an empty database. "
                                   );
            m_access->setLastError(errorMsg);

            if (m_observer)
            {
                m_observer->error(errorMsg);
                m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        // current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.
        m_currentVersion = version.toInt();

        if (m_currentVersion > schemaVersion())
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
                m_access->setLastError(errorMsg);

                if (m_observer)
                {
                    m_observer->error(errorMsg);
                    m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
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

        DbEngineParameters parameters = m_access->parameters();

        // No legacy handling: start with a fresh db
        if (!createDatabase())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n ") +
                               m_access->backend()->lastError();
            m_access->setLastError(errorMsg);

            if (m_observer)
            {
                m_observer->error(errorMsg);
                m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
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
        m_currentVersion         = schemaVersion();
        m_currentRequiredVersion = 1;
        return true;
    }
    else
    {
        return false;
    }
}

bool SimilarityDbSchemaUpdater::createTables()
{
    return m_access->backend()->execDBAction(m_access->backend()->getDBAction(QLatin1String("CreateSimilarityDB")));
}

bool SimilarityDbSchemaUpdater::createIndices()
{
    return m_access->backend()->execDBAction(m_access->backend()->getDBAction(QLatin1String("CreateSimilarityDBIndices")));
}

bool SimilarityDbSchemaUpdater::createTriggers()
{
    return m_access->backend()->execDBAction(m_access->backend()->getDBAction(QLatin1String("CreateSimilarityDBTrigger")));
}

}  // namespace Digikam
