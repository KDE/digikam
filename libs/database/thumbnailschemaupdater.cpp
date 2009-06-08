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

#include <kdebug.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

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
    return 1;
}

ThumbnailSchemaUpdater::ThumbnailSchemaUpdater(ThumbnailDatabaseAccess *access)
{
    m_access         = access;
    m_currentVersion = 0;
    m_observer       = 0;
    m_setError       = false;
}

bool ThumbnailSchemaUpdater::update()
{
    bool success = startUpdates();
    // even on failure, try to set current version - it may have incremented
    m_access->db()->setSetting("DBVersion",QString::number(m_currentVersion));
    return success;
}

void ThumbnailSchemaUpdater::setObserver(InitializationObserver *observer)
{
    m_observer = observer;
}

bool ThumbnailSchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = m_access->backend()->tables();

    if (tables.contains("Thumbnails"))
    {
        // Find out schema version of db file
        QString version = m_access->db()->getSetting("DBVersion");
        QString versionRequired = m_access->db()->getSetting("DBVersionRequired");
        //kDebug(50003) << "Have a database structure version " << version << endl;

        // We absolutely require the DBVersion setting
        if (version.isEmpty())
        {
            // Something is damaged. Give up.
            kError(50003) << "DBVersion not available! Giving up schema upgrading." << endl;
            QString errorMsg = i18n(
                    "The database is not valid: "
                    "the \"DBVersion\" setting does not exist. "
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
            return makeUpdates();
    }
    else
    {
        //kDebug(50003) << "No database file available" << endl;
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
    //DatabaseTransaction transaction(m_access);
    if (m_currentVersion < schemaVersion())
    {
        return false;
    }
    return true;
}


bool ThumbnailSchemaUpdater::createDatabase()
{
    if ( createTablesV1()
         && createIndicesV1()
         && createTriggersV1())
    {
        m_currentVersion = 5;
        return true;
    }
    else
        return false;
}

bool ThumbnailSchemaUpdater::createTablesV1()
{
    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE Thumbnails "
                            "(id INTEGER PRIMARY KEY, "
                            " type INTEGER, "
                            " modificationDate DATETIME, "
                            " orientationHint INTEGER, "
                            " data BLOB);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE UniqueHashes "
                            "(uniqueHash TEXT, "
                            " fileSize INTEGER, "
                            " thumbId INTEGER, "
                            " UNIQUE(uniqueHash, fileSize))") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE FilePaths "
                            "(path TEXT, "
                            " thumbId INTEGER, "
                            " UNIQUE(path));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE Settings         \n"
                            "(keyword TEXT NOT NULL UNIQUE,\n"
                            " value TEXT);") ))
    {
        return false;
    }

    return true;
}

bool ThumbnailSchemaUpdater::createIndicesV1()
{
    m_access->backend()->execSql("CREATE INDEX id_uniqueHashes ON UniqueHashes (thumbId);");
    m_access->backend()->execSql("CREATE INDEX id_filePaths ON FilePaths (thumbId);");
    return true;
}

bool ThumbnailSchemaUpdater::createTriggersV1()
{
    m_access->backend()->execSql("CREATE TRIGGER delete_thumbnails DELETE ON Thumbnails "
                                 "BEGIN "
                                 " DELETE FROM UniqueHashes WHERE UniqueHashes.thumbId = OLD.id; "
                                 " DELETE FROM FilePaths WHERE FilePaths.thumbId = OLD.id; "
                                 "END;");
    return true;
}

}  // namespace Digikam
