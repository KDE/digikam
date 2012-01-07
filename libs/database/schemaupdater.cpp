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

#include "schemaupdater.h"

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

// LibKDcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Local includes

#include "databasebackend.h"
#include "databasetransaction.h"
#include "databasechecker.h"
#include "upgradedb_sqlite2tosqlite3.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionscanner.h"
#include "imagequerybuilder.h"
#include "collectionscannerobserver.h"

namespace Digikam
{

int SchemaUpdater::schemaVersion()
{
    return 6;
}

int SchemaUpdater::filterSettingsVersion()
{
    return 3;
}

int SchemaUpdater::uniqueHashVersion()
{
    return 2;
}

bool SchemaUpdater::isUniqueHashUpToDate()
{
    return DatabaseAccess().db()->getUniqueHashVersion() >= uniqueHashVersion();
}

const QString SchemaUpdater::getLastErrorMessage()
{
    return m_LastErrorMessage;
}

void SchemaUpdater::setDatabaseAccess(DatabaseAccess* access)
{
    m_access=access;
}

SchemaUpdater::SchemaUpdater(AlbumDB* albumDB, DatabaseBackend* backend, DatabaseParameters parameters)
{
    m_Backend         = backend;
    m_AlbumDB         = albumDB;
    m_Parameters      = parameters;
    m_observer        = 0;
    m_setError        = false;
}

bool SchemaUpdater::update()
{
    kDebug() << "SchemaUpdater update";
    bool success = startUpdates();

    // cancelled?
    if (m_observer && !m_observer->continueQuery())
    {
        return false;
    }

    // even on failure, try to set current version - it may have incremented
    setVersionSettings();

    if (!success)
    {
        return false;
    }

    updateFilterSettings();

    if (m_observer)
    {
        m_observer->finishedSchemaUpdate(InitializationObserver::UpdateSuccess);
    }

    return success;
}

void SchemaUpdater::setVersionSettings()
{
    if (m_currentVersion.isValid())
    {
        m_AlbumDB->setSetting("DBVersion", QString::number(m_currentVersion.toInt()));
    }

    if (m_currentRequiredVersion.isValid())
    {
        m_AlbumDB->setSetting("DBVersionRequired", QString::number(m_currentRequiredVersion.toInt()));
    }
}

static QVariant safeToVariant(const QString& s)
{
    if (s.isEmpty())
    {
        return QVariant();
    }
    else
    {
        return s.toInt();
    }
}

void SchemaUpdater::readVersionSettings()
{
    m_currentVersion         = safeToVariant(m_AlbumDB->getSetting("DBVersion"));
    m_currentRequiredVersion = safeToVariant(m_AlbumDB->getSetting("DBVersionRequired"));
}

void SchemaUpdater::setObserver(InitializationObserver* observer)
{
    m_observer = observer;
}

bool SchemaUpdater::startUpdates()
{
    if (!m_Parameters.isImgSQLite())
    {
        // Do we have sufficient privileges
        QStringList insufficientRights;
        DatabasePrivilegesChecker checker(m_Parameters);

        if (!checker.checkPrivileges(insufficientRights))
        {
            kError() << "Insufficient rights on databse.";
            QString errorMsg = i18n(
                                "You have insufficient privileges on the database.\n"
                                "Following privileges are not assigned to you:\n %1"
                                "\nCheck your privileges on the database and restart digiKam.",
                                insufficientRights.join(",\n")
                            );

            m_LastErrorMessage=errorMsg;

            if (m_observer)
            {
                m_observer->error(errorMsg);
                m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }
    }

    // First step: do we have an empty database?
    QStringList tables = m_Backend->tables();

    if (tables.contains("Albums", Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        readVersionSettings();
        kDebug() << "Have a database structure version " << m_currentVersion.toInt();

        // We absolutely require the DBVersion setting
        if (!m_currentVersion.isValid())
        {
            // Something is damaged. Give up.
            kError() << "DBVersion not available! Giving up schema upgrading.";
            QString errorMsg = i18n(
                                   "The database is not valid: "
                                   "the \"DBVersion\" setting does not exist. "
                                   "The current database schema version cannot be verified. "
                                   "Try to start with an empty database. "
                               );
            m_LastErrorMessage=errorMsg;

            if (m_observer)
            {
                m_observer->error(errorMsg);
                m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        // current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.
        if (m_currentVersion.toInt() > schemaVersion())
        {
            // trying to open a database with a more advanced than this SchemaUpdater supports
            if (m_currentRequiredVersion.isValid() && m_currentRequiredVersion.toInt() <= schemaVersion())
            {
                // version required may be less than current version
                return true;
            }
            else
            {
                QString errorMsg = i18n(
                                       "The database has been used with a more recent version of digiKam "
                                       "and has been updated to a database schema which cannot be used with this version. "
                                       "(This means this digiKam version is too old, or the database format is too recent.) "
                                       "Please use the more recent version of digiKam that you used before. "
                                   );
                m_LastErrorMessage=errorMsg;

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
        kDebug() << "No database file available";
        // Legacy handling?

        // first test if there are older files that need to be upgraded.
        // This applies to "digikam.db" for 0.7 and "digikam3.db" for 0.8 and 0.9,
        // all only SQLite databases.

        // Version numbers used in this source file are a bit confused for the historic versions.
        // Version 1 is 0.6 (no db), Version 2 is 0.7 (SQLite 2),
        // Version 3 is 0.8-0.9,
        // Version 3 wrote the setting "DBVersion", "1",
        // Version 4 is 0.10, the digikam3.db file copied to digikam4.db,
        //  no schema changes.
        // Version 4 writes "4", and from now on version x writes "x".
        // Version 5 includes the schema changes from 0.9 to 0.10

        if (m_Parameters.isImgSQLite())
        {
            QFileInfo currentDBFile(m_Parameters.imgDatabaseName);
            QFileInfo digikam3DB(currentDBFile.dir(), "digikam3.db");
            QFileInfo digikamDB(currentDBFile.dir(), "digikam.db");

            if (digikam3DB.exists())
            {
                if (!copyV3toV4(digikam3DB.filePath(), currentDBFile.filePath()))
                {
                    return false;
                }

                // m_currentVersion is now 4;
                return makeUpdates();
            }
            else if (digikamDB.exists())
            {
                if (!updateV2toV4(digikamDB.path()))
                {
                    return false;
                }

                // m_currentVersion is now 4;
                return makeUpdates();
            }

            // no else, fall through!
        }

        // No legacy handling: start with a fresh db
        if (!createDatabase() || !createFilterSettings())
        {
            QString errorMsg = i18n("Failed to create tables in database.\n ")
                               + m_Backend->lastError();
            m_LastErrorMessage=errorMsg;

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

bool SchemaUpdater::beginWrapSchemaUpdateStep()
{
    if (!m_Backend->beginTransaction())
    {
        QFileInfo currentDBFile(m_Parameters.imgDatabaseName);
        QString errorMsg = i18n("Failed to open a database transaction on your database file \"%1\". "
                                "This is unusual. Please check that you can access the file and no "
                                "other process has currently locked the file. "
                                "If the problem persists you can get help from the digikam-devel@kde.org "
                                "mailing list. As well, please have a look at what digiKam prints on the console. ",
                                currentDBFile.filePath());
        m_observer->error(errorMsg);
        m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        return false;
    }

    return true;
}

bool SchemaUpdater::endWrapSchemaUpdateStep(bool stepOperationSuccess, const QString& errorMsg)
{
    if (!stepOperationSuccess)
    {
        m_Backend->rollbackTransaction();

        if (m_observer)
        {
            // error or cancelled?
            if (!m_observer->continueQuery())
            {
                kDebug() << "Schema update cancelled by user";
            }
            else if (!m_setError)
            {
                m_observer->error(errorMsg);
                m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }
        }

        return false;
    }

    kDebug() << "Success updating to v5";
    m_Backend->commitTransaction();
    return true;
}

bool SchemaUpdater::makeUpdates()
{
    kDebug() << "makeUpdates " << m_currentVersion.toInt() << " to " << schemaVersion();

    if (m_currentVersion.toInt() < schemaVersion())
    {
        if (m_currentVersion.toInt() < 5)
        {
            if (!beginWrapSchemaUpdateStep())
            {
                return false;
            }

            // v4 was always SQLite
            QFileInfo currentDBFile(m_Parameters.imgDatabaseName);
            QString errorMsg = i18n("The schema updating process from version 4 to 6 failed, "
                                    "caused by an error that we did not expect. "
                                    "You can try to discard your old database and start with an empty one. "
                                    "(In this case, please move the database files "
                                    "\"%1\" and \"%2\" from the directory \"%3\"). "
                                    "More probably you will want to report this error to the digikam-devel@kde.org "
                                    "mailing list. As well, please have a look at what digiKam prints on the console. ",
                                    QString("digikam3.db"), QString("digikam4.db"), currentDBFile.dir().path());

            if (!endWrapSchemaUpdateStep(updateV4toV6(), errorMsg))
            {
                return false;
            }

            kDebug() << "Success updating v4 to v6";

            // Still set these even in >= 1.4 because 0.10 - 1.3 may want to apply the updates if not set
            setLegacySettingEntries();
        }

        if (m_currentVersion.toInt() < 6)
        {
            //updateV5toV6();
            if (!beginWrapSchemaUpdateStep())
            {
                return false;
            }

            QString errorMsg = i18n("Failed to update the database schema from version 5 to version 6. "
                                    "Please read the error messages printed on the console and "
                                    "report this error as a bug at bugs.kde.org. ");

            if (!endWrapSchemaUpdateStep(updateV5toV6(), errorMsg))
            {
                return false;
            }

            kDebug() << "Success updating to v6";
        }

        // add future updates here
    }

    return true;
}

void SchemaUpdater::defaultFilterSettings(QStringList& defaultImageFilter,
        QStringList& defaultVideoFilter,
        QStringList& defaultAudioFilter)
{
    //NOTE for updating:
    //When changing anything here, just increment filterSettingsVersion() so that the changes take effect

    defaultImageFilter << "jpg" << "jpeg" << "jpe"                    // JPEG
                       << "jp2" << "j2k"  << "jpx"  << "jpc" << "pgx" // JPEG-2000
                       << "tif" << "tiff"                             // TIFF
                       << "png"                                       // PNG
                       << "xpm" << "ppm"  << "pnm" << "pgf"
                       << "gif" << "bmp"  << "xcf" << "pcx";

    defaultImageFilter << KDcrawIface::KDcraw::rawFilesList();

    defaultVideoFilter << "mpeg" << "mpg" << "mpo" << "mpe"     // MPEG
                       << "avi"  << "mov" << "wmf" << "asf" << "mp4" << "3gp" << "wmv";

    defaultAudioFilter << "ogg" << "mp3" << "wma" << "wav";
}

bool SchemaUpdater::createFilterSettings()
{
    QStringList defaultImageFilter, defaultVideoFilter, defaultAudioFilter;
    defaultFilterSettings(defaultImageFilter, defaultVideoFilter, defaultAudioFilter);

    m_AlbumDB->setFilterSettings(defaultImageFilter, defaultVideoFilter, defaultAudioFilter);
    m_AlbumDB->setSetting("FilterSettingsVersion", QString::number(filterSettingsVersion()));
    m_AlbumDB->setSetting("DcrawFilterSettingsVersion", QString::number(KDcrawIface::KDcraw::rawFilesVersion()));

    return true;
}

bool SchemaUpdater::updateFilterSettings()
{
    QString filterVersion = m_AlbumDB->getSetting("FilterSettingsVersion");
    QString dcrawFilterVersion = m_AlbumDB->getSetting("DcrawFilterSettingsVersion");

    if (
        filterVersion.toInt() < filterSettingsVersion() ||
        dcrawFilterVersion.toInt() < KDcrawIface::KDcraw::rawFilesVersion()
    )
    {
        createFilterSettings();
    }

    return true;
}

bool SchemaUpdater::createDatabase()
{
    if ( createTables()
         && createIndices()
         && createTriggers())
    {
        setLegacySettingEntries();

        m_currentVersion = schemaVersion();

        // if we start with the V2 hash, version 6 is required
        m_AlbumDB->setUniqueHashVersion(uniqueHashVersion());
        m_currentRequiredVersion = schemaVersion();
        /*
        // Digikam for database version 5 can work with version 6, though not using the new features
        m_currentRequiredVersion = 5;
        */
        return true;
    }
    else
    {
        return false;
    }
}

bool SchemaUpdater::createTables()
{
    return m_Backend->execDBAction(m_Backend->getDBAction("CreateDB"));
}

bool SchemaUpdater::createIndices()
{
    // TODO: see which more indices are needed
    // create indices
    return m_Backend->execDBAction(m_Backend->getDBAction("CreateIndices"));
}

bool SchemaUpdater::createTriggers()
{
    return m_Backend->execDBAction(m_Backend->getDBAction(QString("CreateTriggers")));
}

bool SchemaUpdater::updateUniqueHash()
{
    if (isUniqueHashUpToDate())
    {
        return true;
    }

    readVersionSettings();

    {
        DatabaseTransaction transaction;

        DatabaseAccess().db()->setUniqueHashVersion(uniqueHashVersion());

        CollectionScanner scanner;
        scanner.setNeedFileCount(true);
        scanner.setUpdateHashHint();
        if (m_observer)
        {
            m_observer->connectCollectionScanner(&scanner);
            scanner.setObserver(m_observer);
        }
        scanner.completeScan();

        // earlier digikam does not know about the hash
        if (m_currentRequiredVersion.toInt() < 6)
        {
            m_currentRequiredVersion = 6;
            setVersionSettings();
        }
    }
    return true;
}

bool SchemaUpdater::updateV5toV6()
{
    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->moreSchemaUpdateSteps(1);
    }

    DatabaseAction updateAction = m_Backend->getDBAction("UpdateSchemaFromV5ToV6");
    if (updateAction.name.isNull())
    {
        QString errorMsg = i18n("The database update action cannot be found. Please ensure that "
                                "the dbconfig.xml file of the current version of digiKam is installed "
                                "at the correct place. ");
    }

    if (!m_Backend->execDBAction(updateAction))
    {
        kError() << "Schema update to V6 failed!";
        // resort to default error message, set above
        return false;
    }

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Updated schema to version 6."));
    }

    m_currentVersion = 6;
    // Digikam for database version 5 can work with version 6, though not using the new features
    // Note: We do not upgrade the uniqueHash
    m_currentRequiredVersion = 5;
    return true;
}

bool SchemaUpdater::copyV3toV4(const QString& digikam3DBPath, const QString& currentDBPath)
{
    if (m_observer)
    {
        m_observer->moreSchemaUpdateSteps(2);
    }

    m_Backend->close();

    // We cannot use KIO here because KIO only works from the main thread
    QFile oldFile(digikam3DBPath);
    QFile newFile(currentDBPath);
    // QFile won't override. Remove the empty db file created when a non-existent file is opened
    newFile.remove();

    if (!oldFile.copy(currentDBPath))
    {
        QString errorMsg = i18n("Failed to copy the old database file (\"%1\") "
                                "to its new location (\"%2\"). "
                                "Error message: \"%3\". "
                                "Please make sure that the file can be copied, "
                                "or delete it.",
                                digikam3DBPath, currentDBPath, oldFile.errorString());
        m_LastErrorMessage=errorMsg;
        m_setError = true;

        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (m_observer)
    {
        m_observer->schemaUpdateProgress(i18n("Copied database file"));
    }

    if (!m_Backend->open(m_Parameters))
    {
        QString errorMsg = i18n("The old database file (\"%1\") has been copied "
                                "to the new location (\"%2\") but it cannot be opened. "
                                "Please delete both files and try again, "
                                "starting with an empty database. ",
                                digikam3DBPath, currentDBPath);

        m_LastErrorMessage=errorMsg;
        m_setError = true;

        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (m_observer)
    {
        m_observer->schemaUpdateProgress(i18n("Opened new database file"));
    }

    m_currentVersion = 4;
    return true;
}

bool SchemaUpdater::updateV2toV4(const QString& sqlite2DBPath)
{
    if (m_observer)
    {
        m_observer->moreSchemaUpdateSteps(1);
    }

    if (upgradeDB_Sqlite2ToSqlite3(m_AlbumDB, m_Backend, sqlite2DBPath))
    {
        m_currentVersion = 4;
        return true;
    }
    else
    {
        QString errorMsg = i18n("Could not update from the old SQLite2 file (\"%1\"). "
                                "Please delete this file and try again, "
                                "starting with an empty database. ", sqlite2DBPath);
        m_LastErrorMessage=errorMsg;

        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    // FIXME: We are not returning anything, if we land in this section of the code!
    if (m_observer)
    {
        m_observer->schemaUpdateProgress(i18n("Updated from 0.7 database"));
    }
}

static QStringList cleanUserFilterString(const QString& filterString)
{
    // splits by either ; or space, removes "*.", trims
    QStringList filterList;

    QString wildcard("*.");
    QChar dot('.');

    QChar sep(';');
    int i = filterString.indexOf( sep );

    if ( i == -1 && filterString.indexOf(' ') != -1 )
    {
        sep = QChar(' ');
    }

    QStringList sepList = filterString.split(sep, QString::SkipEmptyParts);
    foreach(const QString& f, sepList)
    {
        if (f.startsWith(wildcard))
        {
            filterList << f.mid(2).trimmed().toLower();
        }
        else
        {
            filterList << f.trimmed().toLower();
        }
    }
    return filterList;
}

bool SchemaUpdater::updateV4toV6()
{
    kDebug() << "updateV4toV6";

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->moreSchemaUpdateSteps(11);
    }

    // This update was introduced from digikam version 0.9 to digikam 0.10
    // We operator on an SQLite3 database under a transaction (which will be rolled back on error)

    // --- Make space for new tables ---
    if (!m_Backend->execSql(QString("ALTER TABLE Albums RENAME TO AlbumsV3;")))
    {
        return false;
    }

    if (!m_Backend->execSql(QString("ALTER TABLE Images RENAME TO ImagesV3;")))
    {
        return false;
    }

    if (!m_Backend->execSql(QString("ALTER TABLE Searches RENAME TO SearchesV3;")))
    {
        return false;
    }

    kDebug() << "Moved tables";
    // --- Drop some triggers and indices ---

    // Don't check for errors here. The "IF EXISTS" clauses seem not supported in SQLite
    m_Backend->execSql(QString("DROP TRIGGER delete_album;"));
    m_Backend->execSql(QString("DROP TRIGGER delete_image;"));
    m_Backend->execSql(QString("DROP TRIGGER delete_tag;"));
    m_Backend->execSql(QString("DROP TRIGGER insert_tagstree;"));
    m_Backend->execSql(QString("DROP TRIGGER delete_tagstree;"));
    m_Backend->execSql(QString("DROP TRIGGER move_tagstree;"));
    m_Backend->execSql(QString("DROP INDEX dir_index;"));
    m_Backend->execSql(QString("DROP INDEX tag_index;"));

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Prepared table creation"));
    }

    kDebug() << "Dropped triggers";

    // --- Create new tables ---

    if (!createTables() || !createIndices())
    {
        return false;
    }

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Created tables"));
    }

    // --- Populate AlbumRoots (from config) ---

    KSharedConfigPtr config = KGlobal::config();

    KConfigGroup group = config->group("Album Settings");
    QString albumLibraryPath = group.readEntry("Album Path", QString());

    if (albumLibraryPath.isEmpty())
    {
        kError() << "Album library path from config file is empty. Aborting update.";
        QString errorMsg = i18n("No album library path has been found in the configuration file. "
                                "Giving up the schema updating process. "
                                "Please try with an empty database, or repair your configuration.");
        m_LastErrorMessage=errorMsg;
        m_setError = true;

        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    CollectionLocation location =
        CollectionManager::instance()->addLocation(KUrl::fromPath(albumLibraryPath));

    if (location.isNull())
    {
        kError() << "Failure to create a collection location. Aborting update.";
        QString errorMsg = i18n("There was an error associating your albumLibraryPath (\"%1\") "
                                "with a storage volume of your system. "
                                "This problem may indicate that there is a problem with your installation. "
                                "If you are working on Linux, check that HAL is installed and running. "
                                "In any case, you can seek advice from the digikam-devel@kde.org mailing list. "
                                "The database updating process will now be aborted because we do not want "
                                "to create a new database based on false assumptions from a broken installation.",
                                albumLibraryPath);
        m_LastErrorMessage=errorMsg;
        m_setError = true;

        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Configured one album root"));
    }

    kDebug() << "Inserted album root";

    // --- With the album root, populate albums ---

    if (!m_Backend->execSql(QString(
                                "REPLACE INTO Albums "
                                " (id, albumRoot, relativePath, date, caption, collection, icon) "
                                "SELECT id, ?, url, date, caption, collection, icon "
                                " FROM AlbumsV3;"
                            ),
                            location.id())
       )
    {
        return false;
    }

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Imported albums"));
    }

    kDebug() << "Populated albums";

    // --- Add images ---

    if (!m_Backend->execSql(QString(
                                "REPLACE INTO Images "
                                " (id, album, name, status, category, modificationDate, fileSize, uniqueHash) "
                                "SELECT id, dirid, name, ?, ?, NULL, NULL, NULL"
                                " FROM ImagesV3;"
                            ),
                            DatabaseItem::Visible, DatabaseItem::UndefinedCategory)
       )
    {
        return false;
    }

    if (!m_access->backend()->execSql(QString(
                                          "REPLACE INTO ImageInformation (imageId) SELECT id FROM Images;"))
       )
    {
        return false;
    }

    // remove orphan images that would not be removed by CollectionScanner
    m_Backend->execSql(QString("DELETE FROM Images WHERE album NOT IN (SELECT id FROM Albums);"));

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Imported images information"));
    }

    kDebug() << "Populated Images";

    // --- Port searches ---

    if (!m_Backend->execSql(QString(
                                "REPLACE INTO Searches "
                                " (id, type, name, query) "
                                "SELECT id, ?, name, url"
                                " FROM SearchesV3;"),
                            DatabaseSearch::LegacyUrlSearch)
       )
    {
        return false;
    }

    SearchInfo::List sList = m_AlbumDB->scanSearches();

    for (SearchInfo::List::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
    {
        KUrl url((*it).query);

        ImageQueryBuilder builder;
        QString query = builder.convertFromUrlToXml(url);

        QString name = (*it).name;

        if (name == i18n("Last Search"))
        {
            name = i18n("Last Search (0.9)");
        }

        if (url.queryItem("type") == QString("datesearch"))
        {
            m_AlbumDB->updateSearch((*it).id, DatabaseSearch::TimeLineSearch, name, query);
        }
        else if (url.queryItem("1.key") == "keyword")
        {
            m_AlbumDB->updateSearch((*it).id, DatabaseSearch::KeywordSearch, name, query);
        }
        else
        {
            m_AlbumDB->updateSearch((*it).id, DatabaseSearch::AdvancedSearch, name, query);
        }
    }

    // --- Create triggers ---

    if (!createTriggers())
    {
        return false;
    }

    kDebug() << "Created triggers";

    // --- Populate name filters ---

    createFilterSettings();

    // --- Set user settings from config ---

    QStringList defaultImageFilter, defaultVideoFilter, defaultAudioFilter;
    defaultFilterSettings(defaultImageFilter, defaultVideoFilter, defaultAudioFilter);

    QSet<QString> configImageFilter, configVideoFilter, configAudioFilter;

    configImageFilter   = cleanUserFilterString(group.readEntry("File Filter", QString())).toSet();
    configImageFilter  += cleanUserFilterString(group.readEntry("Raw File Filter", QString())).toSet();
    configVideoFilter   = cleanUserFilterString(group.readEntry("Movie File Filter", QString())).toSet();
    configAudioFilter   = cleanUserFilterString(group.readEntry("Audio File Filter", QString())).toSet();

    // remove those that are included in the default filter
    configImageFilter.subtract(defaultImageFilter.toSet());
    configVideoFilter.subtract(defaultVideoFilter.toSet());
    configAudioFilter.subtract(defaultAudioFilter.toSet());

    m_AlbumDB->setUserFilterSettings(configImageFilter.toList(), configVideoFilter.toList(), configAudioFilter.toList());
    kDebug() << "Set initial filter settings with user settings" << configImageFilter;

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Initialized and imported file suffix filter"));
    }

    // --- do a full scan ---

    CollectionScanner scanner;

    if (m_observer)
    {
        m_observer->connectCollectionScanner(&scanner);
        scanner.setObserver(m_observer);
    }

    scanner.completeScan();

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Did the initial full scan"));
    }

    // --- Port date, comment and rating (_after_ the scan) ---

    // Port ImagesV3.date -> ImageInformation.creationDate
    if (!m_Backend->execSql(QString(
                                "UPDATE ImageInformation SET "
                                " creationDate=(SELECT datetime FROM ImagesV3 WHERE ImagesV3.id=ImageInformation.imageid) "
                                "WHERE imageid IN (SELECT id FROM ImagesV3);"
                            )
                           )
       )
    {
        return false;
    }

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Imported creation dates"));
    }

    // Port ImagesV3.comment to ImageComments

    // An author of NULL will inhibt the UNIQUE restriction to take effect (but #189080). Work around.
    m_Backend->execSql(QString(
                           "DELETE FROM ImageComments WHERE "
                           "type=? AND language=? AND author IS NULL "
                           "AND imageid IN ( SELECT id FROM ImagesV3 ); "),
                       (int)DatabaseComment::Comment, QString("x-default"));

    if (!m_Backend->execSql(QString(
                                "REPLACE INTO ImageComments "
                                " (imageid, type, language, comment) "
                                "SELECT id, ?, ?, caption FROM ImagesV3;"
                            ),
                            (int)DatabaseComment::Comment, QString("x-default"))
       )
    {
        return false;
    }

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Imported comments"));
    }

    // Port rating storage in ImageProperties to ImageInformation
    if (!m_Backend->execSql(QString(
                                "UPDATE ImageInformation SET "
                                " rating=(SELECT value FROM ImageProperties "
                                "         WHERE ImageInformation.imageid=ImageProperties.imageid AND ImageProperties.property=?) "
                                "WHERE imageid IN (SELECT imageid FROM ImageProperties WHERE property=?);"
                            ),
                            QString("Rating"), QString("Rating"))
       )
    {
        return false;
    }

    m_Backend->execSql(QString("DELETE FROM ImageProperties WHERE property=?;"), QString("Rating"));
    m_Backend->execSql(QString("UPDATE ImageInformation SET rating=0 WHERE rating<0;"));

    if (m_observer)
    {
        if (!m_observer->continueQuery())
        {
            return false;
        }

        m_observer->schemaUpdateProgress(i18n("Imported ratings"));
    }

    // --- Drop old tables ---

    m_Backend->execSql(QString("DROP TABLE ImagesV3;"));
    m_Backend->execSql(QString("DROP TABLE AlbumsV3;"));
    m_Backend->execSql(QString("DROP TABLE SearchesV3;"));

    if (m_observer)
    {
        m_observer->schemaUpdateProgress(i18n("Dropped v3 tables"));
    }

    m_currentRequiredVersion = 5;
    m_currentVersion = 6;
    kDebug() << "Returning true from updating to 5";
    return true;
}

void SchemaUpdater::setLegacySettingEntries()
{
    m_AlbumDB->setSetting("preAlpha010Update1", "true");
    m_AlbumDB->setSetting("preAlpha010Update2", "true");
    m_AlbumDB->setSetting("preAlpha010Update3", "true");
    m_AlbumDB->setSetting("beta010Update1", "true");
    m_AlbumDB->setSetting("beta010Update2", "true");
}

// ---------- Legacy code ------------


void SchemaUpdater::preAlpha010Update1()
{
    QString hasUpdate = m_AlbumDB->getSetting("preAlpha010Update1");

    if (!hasUpdate.isNull())
    {
        return;
    }

    if (!m_Backend->execSql(QString("ALTER TABLE Searches RENAME TO SearchesV3;")))
    {
        return;
    }

    if ( !m_Backend->execSql(
             QString( "CREATE TABLE IF NOT EXISTS Searches  \n"
                      " (id INTEGER PRIMARY KEY, \n"
                      "  type INTEGER, \n"
                      "  name TEXT NOT NULL, \n"
                      "  query TEXT NOT NULL);" ) ))
    {
        return;
    }

    if (!m_Backend->execSql(QString(
                                "REPLACE INTO Searches "
                                " (id, type, name, query) "
                                "SELECT id, ?, name, url"
                                " FROM SearchesV3;"),
                            DatabaseSearch::LegacyUrlSearch)
       )
    {
        return;
    }

    SearchInfo::List sList = m_AlbumDB->scanSearches();

    for (SearchInfo::List::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
    {
        KUrl url((*it).query);

        ImageQueryBuilder builder;
        QString query = builder.convertFromUrlToXml(url);

        if (url.queryItem("type") == QString("datesearch"))
        {
            m_AlbumDB->updateSearch((*it).id, DatabaseSearch::TimeLineSearch, (*it).name, query);
        }
        else if (url.queryItem("1.key") == "keyword")
        {
            m_AlbumDB->updateSearch((*it).id, DatabaseSearch::KeywordSearch, (*it).name, query);
        }
        else
        {
            m_AlbumDB->updateSearch((*it).id, DatabaseSearch::AdvancedSearch, (*it).name, query);
        }
    }

    m_Backend->execSql(QString("DROP TABLE SearchesV3;"));

    m_AlbumDB->setSetting("preAlpha010Update1", "true");
}

void SchemaUpdater::preAlpha010Update2()
{
    QString hasUpdate = m_AlbumDB->getSetting("preAlpha010Update2");

    if (!hasUpdate.isNull())
    {
        return;
    }

    if (!m_Backend->execSql(QString("ALTER TABLE ImagePositions RENAME TO ImagePositionsTemp;")))
    {
        return;
    }

    if (!m_Backend->execSql(QString("ALTER TABLE ImageMetadata RENAME TO ImageMetadataTemp;")))
    {
        return;
    }

    m_Backend->execSql(
        QString("CREATE TABLE ImagePositions\n"
                " (imageid INTEGER PRIMARY KEY,\n"
                "  latitude TEXT,\n"
                "  latitudeNumber REAL,\n"
                "  longitude TEXT,\n"
                "  longitudeNumber REAL,\n"
                "  altitude REAL,\n"
                "  orientation REAL,\n"
                "  tilt REAL,\n"
                "  roll REAL,\n"
                "  accuracy REAL,\n"
                "  description TEXT);") );

    m_Backend->execSql(QString(
                           "REPLACE INTO ImagePositions "
                           " (imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                           "  altitude, orientation, tilt, roll, accuracy, description) "
                           "SELECT imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                           "  altitude, orientation, tilt, roll, 0, description "
                           " FROM ImagePositionsTemp;"));

    m_Backend->execSql(
        QString("CREATE TABLE ImageMetadata\n"
                " (imageid INTEGER PRIMARY KEY,\n"
                "  make TEXT,\n"
                "  model TEXT,\n"
                "  lens TEXT,\n"
                "  aperture REAL,\n"
                "  focalLength REAL,\n"
                "  focalLength35 REAL,\n"
                "  exposureTime REAL,\n"
                "  exposureProgram INTEGER,\n"
                "  exposureMode INTEGER,\n"
                "  sensitivity INTEGER,\n"
                "  flash INTEGER,\n"
                "  whiteBalance INTEGER,\n"
                "  whiteBalanceColorTemperature INTEGER,\n"
                "  meteringMode INTEGER,\n"
                "  subjectDistance REAL,\n"
                "  subjectDistanceCategory INTEGER);") );

    m_Backend->execSql( QString("INSERT INTO ImageMetadata "
                                " (imageid, make, model, lens, aperture, focalLength, focalLength35, "
                                "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                                "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) "
                                "SELECT imageid, make, model, NULL, aperture, focalLength, focalLength35, "
                                "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                                "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory "
                                "FROM ImageMetadataTemp;"));

    m_Backend->execSql(QString("DROP TABLE ImagePositionsTemp;"));
    m_Backend->execSql(QString("DROP TABLE ImageMetadataTemp;"));

    m_AlbumDB->setSetting("preAlpha010Update2", "true");
}

void SchemaUpdater::preAlpha010Update3()
{
    QString hasUpdate = m_AlbumDB->getSetting("preAlpha010Update3");

    if (!hasUpdate.isNull())
    {
        return;
    }

    m_Backend->execSql(QString("DROP TABLE ImageCopyright;"));
    m_Backend->execSql(
        QString("CREATE TABLE ImageCopyright\n"
                " (imageid INTEGER,\n"
                "  property TEXT,\n"
                "  value TEXT,\n"
                "  extraValue TEXT,\n"
                "  UNIQUE(imageid, property, value, extraValue));")
    );

    m_AlbumDB->setSetting("preAlpha010Update3", "true");
}

void SchemaUpdater::beta010Update1()
{
    QString hasUpdate = m_AlbumDB->getSetting("beta010Update1");

    if (!hasUpdate.isNull())
    {
        return;
    }

    // if Image has been deleted
    m_Backend->execSql("DROP TRIGGER delete_image;");
    m_Backend->execSql(
        "CREATE TRIGGER delete_image DELETE ON Images\n"
        "BEGIN\n"
        "  DELETE FROM ImageTags\n"
        "    WHERE imageid=OLD.id;\n"
        "  DELETE From ImageHaarMatrix\n "
        "    WHERE imageid=OLD.id;\n"
        "  DELETE From ImageInformation\n "
        "    WHERE imageid=OLD.id;\n"
        "  DELETE From ImageMetadata\n "
        "    WHERE imageid=OLD.id;\n"
        "  DELETE From ImagePositions\n "
        "    WHERE imageid=OLD.id;\n"
        "  DELETE From ImageComments\n "
        "    WHERE imageid=OLD.id;\n"
        "  DELETE From ImageCopyright\n "
        "    WHERE imageid=OLD.id;\n"
        "  DELETE From ImageProperties\n "
        "    WHERE imageid=OLD.id;\n"
        "  UPDATE Albums SET icon=null \n "
        "    WHERE icon=OLD.id;\n"
        "  UPDATE Tags SET icon=null \n "
        "    WHERE icon=OLD.id;\n"
        "END;");


    m_AlbumDB->setSetting("beta010Update1", "true");
}

void SchemaUpdater::beta010Update2()
{
    QString hasUpdate = m_AlbumDB->getSetting("beta010Update2");

    if (!hasUpdate.isNull())
    {
        return;
    }

    // force rescan and creation of ImageInformation entry for videos and audio
    m_Backend->execSql("DELETE FROM Images WHERE category=2 OR category=3;");

    m_AlbumDB->setSetting("beta010Update2", "true");
}

bool SchemaUpdater::createTablesV3()
{
    if (!m_Backend->execSql( QString("CREATE TABLE Albums\n"
                                     " (id INTEGER PRIMARY KEY,\n"
                                     "  url TEXT NOT NULL UNIQUE,\n"
                                     "  date DATE NOT NULL,\n"
                                     "  caption TEXT,\n"
                                     "  collection TEXT,\n"
                                     "  icon INTEGER);") ))
    {
        return false;
    }

    if (!m_Backend->execSql( QString("CREATE TABLE Tags\n"
                                     " (id INTEGER PRIMARY KEY,\n"
                                     "  pid INTEGER,\n"
                                     "  name TEXT NOT NULL,\n"
                                     "  icon INTEGER,\n"
                                     "  iconkde TEXT,\n"
                                     "  UNIQUE (name, pid));") ))
    {
        return false;
    }

    if (!m_Backend->execSql( QString("CREATE TABLE TagsTree\n"
                                     " (id INTEGER NOT NULL,\n"
                                     "  pid INTEGER NOT NULL,\n"
                                     "  UNIQUE (id, pid));") ))
    {
        return false;
    }

    if (!m_Backend->execSql( QString("CREATE TABLE Images\n"
                                     " (id INTEGER PRIMARY KEY,\n"
                                     "  name TEXT NOT NULL,\n"
                                     "  dirid INTEGER NOT NULL,\n"
                                     "  caption TEXT,\n"
                                     "  datetime DATETIME,\n"
                                     "  UNIQUE (name, dirid));") ))
    {
        return false;
    }


    if (!m_Backend->execSql( QString("CREATE TABLE ImageTags\n"
                                     " (imageid INTEGER NOT NULL,\n"
                                     "  tagid INTEGER NOT NULL,\n"
                                     "  UNIQUE (imageid, tagid));") ))
    {
        return false;
    }

    if (!m_Backend->execSql( QString("CREATE TABLE ImageProperties\n"
                                     " (imageid  INTEGER NOT NULL,\n"
                                     "  property TEXT    NOT NULL,\n"
                                     "  value    TEXT    NOT NULL,\n"
                                     "  UNIQUE (imageid, property));") ))
    {
        return false;
    }

    if ( !m_Backend->execSql( QString( "CREATE TABLE Searches  \n"
                                       " (id INTEGER PRIMARY KEY, \n"
                                       "  name TEXT NOT NULL UNIQUE, \n"
                                       "  url  TEXT NOT NULL);" ) ) )
    {
        return false;
    }

    if (!m_Backend->execSql( QString("CREATE TABLE Settings         \n"
                                     "(keyword TEXT NOT NULL UNIQUE,\n"
                                     " value TEXT);") ))
    {
        return false;
    }

    // TODO: see which more indices are needed
    // create indices
    m_Backend->execSql("CREATE INDEX dir_index ON Images    (dirid);");
    m_Backend->execSql("CREATE INDEX tag_index ON ImageTags (tagid);");

    // create triggers

    // trigger: delete from Images/ImageTags/ImageProperties
    // if Album has been deleted
    m_Backend->execSql("CREATE TRIGGER delete_album DELETE ON Albums\n"
                       "BEGIN\n"
                       " DELETE FROM ImageTags\n"
                       "   WHERE imageid IN (SELECT id FROM Images WHERE dirid=OLD.id);\n"
                       " DELETE From ImageProperties\n"
                       "   WHERE imageid IN (SELECT id FROM Images WHERE dirid=OLD.id);\n"
                       " DELETE FROM Images\n"
                       "   WHERE dirid = OLD.id;\n"
                       "END;");

    // trigger: delete from ImageTags/ImageProperties
    // if Image has been deleted
    m_Backend->execSql("CREATE TRIGGER delete_image DELETE ON Images\n"
                       "BEGIN\n"
                       "  DELETE FROM ImageTags\n"
                       "    WHERE imageid=OLD.id;\n"
                       "  DELETE From ImageProperties\n "
                       "    WHERE imageid=OLD.id;\n"
                       "  UPDATE Albums SET icon=null \n "
                       "    WHERE icon=OLD.id;\n"
                       "  UPDATE Tags SET icon=null \n "
                       "    WHERE icon=OLD.id;\n"
                       "END;");

    // trigger: delete from ImageTags if Tag has been deleted
    m_Backend->execSql("CREATE TRIGGER delete_tag DELETE ON Tags\n"
                       "BEGIN\n"
                       "  DELETE FROM ImageTags WHERE tagid=OLD.id;\n"
                       "END;");

    // trigger: insert into TagsTree if Tag has been added
    m_Backend->execSql("CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags\n"
                       "BEGIN\n"
                       "  INSERT INTO TagsTree\n"
                       "    SELECT NEW.id, NEW.pid\n"
                       "    UNION\n"
                       "    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;\n"
                       "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    m_Backend->execSql("CREATE TRIGGER delete_tagstree DELETE ON Tags\n"
                       "BEGIN\n"
                       " DELETE FROM Tags\n"
                       "   WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
                       " DELETE FROM TagsTree\n"
                       "   WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
                       " DELETE FROM TagsTree\n"
                       "    WHERE id=OLD.id;\n"
                       "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    m_Backend->execSql("CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags\n"
                       "BEGIN\n"
                       "  DELETE FROM TagsTree\n"
                       "    WHERE\n"
                       "      ((id = OLD.id)\n"
                       "        OR\n"
                       "        id IN (SELECT id FROM TagsTree WHERE pid=OLD.id))\n"
                       "      AND\n"
                       "      pid IN (SELECT pid FROM TagsTree WHERE id=OLD.id);\n"
                       "  INSERT INTO TagsTree\n"
                       "     SELECT NEW.id, NEW.pid\n"
                       "     UNION\n"
                       "     SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid\n"
                       "     UNION\n"
                       "     SELECT id, NEW.pid FROM TagsTree WHERE pid=NEW.id\n"
                       "     UNION\n"
                       "     SELECT A.id, B.pid FROM TagsTree A, TagsTree B\n"
                       "        WHERE\n"
                       "        A.pid = NEW.id AND B.id = NEW.pid;\n"
                       "END;");

    return true;
}

}  // namespace Digikam
