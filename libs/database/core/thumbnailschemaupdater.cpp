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

#include "thumbnailschemaupdater.h"

// Qt includes

#include <QFileInfo>
#include <QFile>
#include <QDir>

// KDE includes

#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

// Local includes

#include "collectionscannerobserver.h"
#include "databasecorebackend.h"
#include "databasetransaction.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"

namespace Digikam
{

int ThumbnailSchemaUpdater::schemaVersion()
{
    return 2;
}

ThumbnailSchemaUpdater::ThumbnailSchemaUpdater(ThumbnailDatabaseAccess* access)
{
    m_access         = access;
    m_currentVersion = 0;
    m_currentRequiredVersion = 0;
    m_observer       = 0;
    m_setError       = false;
}

bool ThumbnailSchemaUpdater::update()
{
    bool success = startUpdates();

    // even on failure, try to set current version - it may have incremented
    if (m_currentVersion)
    {
        m_access->db()->setSetting("DBThumbnailsVersion", QString::number(m_currentVersion));
    }

    if (m_currentRequiredVersion)
    {
        m_access->db()->setSetting("DBThumbnailsVersionRequired", QString::number(m_currentRequiredVersion));
    }

    return success;
}

void ThumbnailSchemaUpdater::setObserver(InitializationObserver* observer)
{
    m_observer = observer;
}

bool ThumbnailSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = m_access->backend()->tables();

    if (tables.contains("Thumbnails", Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        QString version = m_access->db()->getSetting("DBThumbnailsVersion");
        QString versionRequired = m_access->db()->getSetting("DBThumbnailsVersionRequired");
        kDebug(50003) << "Have a thumbnail database structure version " << version;

        // mini schema update
        if (version.isEmpty() && m_access->parameters().isSQLite())
        {
            version = m_access->db()->getSetting("DBVersion");
        }

        // We absolutely require the DBThumbnailsVersion setting
        if (version.isEmpty())
        {
            // Something is damaged. Give up.
            kError(50003) << "DBThumbnailsVersion not available! Giving up schema upgrading.";
            QString errorMsg = i18n(
                                   "The database is not valid: "
                                   "the \"DBThumbnailsVersion\" setting does not exist. "
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
            // trying to open a database with a more advanced than this ThumbnailSchemaUpdater supports
            if (!versionRequired.isEmpty() && versionRequired.toInt() <= schemaVersion())
            {
                // version required may be less than current version
                return true;
            }
            else
            {
                QString errorMsg = i18n(
                                       "The database has been used with a more recent version of digiKam "
                                       "and has been updated to a database schema which cannot be used with this version. "
                                       "(This means this digiKam version is too old, or the database format is to recent) "
                                       "Please use the more recent version of digikam that you used before. "
                                   );
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
        //kDebug(50003) << "No database file available";
        DatabaseParameters parameters = m_access->parameters();

        // No legacy handling: start with a fresh db
        if (!createDatabase())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n ")
                               + m_access->backend()->lastError();
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

bool ThumbnailSchemaUpdater::makeUpdates()
{
    if (m_currentVersion < schemaVersion())
    {
        if (m_currentVersion == 1)
        {
            updateV1ToV2();
        }
    }

    return true;
}


bool ThumbnailSchemaUpdater::createDatabase()
{
    if ( createTables()
         && createIndices()
         && createTriggers())
    {
        m_currentVersion = schemaVersion();
        m_currentRequiredVersion = 1;
        return true;
    }
    else
    {
        return false;
    }
}

bool ThumbnailSchemaUpdater::createTables()
{
    return m_access->backend()->execDBAction(m_access->backend()->getDBAction(QString("CreateThumbnailsDB")));
}

bool ThumbnailSchemaUpdater::createIndices()
{
    return m_access->backend()->execDBAction(m_access->backend()->getDBAction("CreateThumbnailsDBIndices"));
}

bool ThumbnailSchemaUpdater::createTriggers()
{
    return m_access->backend()->execDBAction(m_access->backend()->getDBAction("CreateThumbnailsDBTrigger"));
}

bool ThumbnailSchemaUpdater::updateV1ToV2()
{
    if (!m_access->backend()->execDBAction(m_access->backend()->getDBAction("UpdateThumbnailsDBSchemaFromV1ToV2")))
    {
        kError() << "Schema upgrade in ThumbnailDB from V1 to V2 failed!";
        return false;
    }

    m_currentVersion = 2;
    m_currentRequiredVersion = 1;
    return true;
}

}  // namespace Digikam
