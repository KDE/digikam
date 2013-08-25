/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionscanner.h"
#include "imagequerybuilder.h"
#include "collectionscannerobserver.h"
#include "config-digikam.h"
#ifdef HAVE_SQLITE2
#include "upgradedb_sqlite2tosqlite3.h"
#endif

namespace Digikam
{

int SchemaUpdater::schemaVersion()
{
    return 7;
}

int SchemaUpdater::filterSettingsVersion()
{
    return 4;
}

int SchemaUpdater::uniqueHashVersion()
{
    return 2;
}

bool SchemaUpdater::isUniqueHashUpToDate()
{
    return DatabaseAccess().db()->getUniqueHashVersion() >= uniqueHashVersion();
}

// --------------------------------------------------------------------------------------

class SchemaUpdater::Private
{

public:

    Private()
    {
        setError = false;
        backend  = 0;
        albumDB  = 0;
        access   = 0;
        observer = 0;
    }

    bool                    setError;

    QVariant                currentVersion;
    QVariant                currentRequiredVersion;

    DatabaseBackend*        backend;
    AlbumDB*                albumDB;
    DatabaseParameters      parameters;

    // legacy
    DatabaseAccess*         access;

    QString                 lastErrorMessage;
    InitializationObserver* observer;
};

SchemaUpdater::SchemaUpdater(AlbumDB* const albumDB, DatabaseBackend* const backend, DatabaseParameters parameters)
    : d(new Private)
{
    d->backend    = backend;
    d->albumDB    = albumDB;
    d->parameters = parameters;

}

SchemaUpdater::~SchemaUpdater()
{
    delete d;
}

void SchemaUpdater::setDatabaseAccess(DatabaseAccess* const access)
{
    d->access = access;
}

const QString SchemaUpdater::getLastErrorMessage()
{
    return d->lastErrorMessage;
}

bool SchemaUpdater::update()
{
    kDebug() << "SchemaUpdater update";
    bool success = startUpdates();

    // cancelled?
    if (d->observer && !d->observer->continueQuery())
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

    if (d->observer)
    {
        d->observer->finishedSchemaUpdate(InitializationObserver::UpdateSuccess);
    }

    return success;
}

void SchemaUpdater::setVersionSettings()
{
    if (d->currentVersion.isValid())
    {
        d->albumDB->setSetting("DBVersion", QString::number(d->currentVersion.toInt()));
    }

    if (d->currentRequiredVersion.isValid())
    {
        d->albumDB->setSetting("DBVersionRequired", QString::number(d->currentRequiredVersion.toInt()));
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
    d->currentVersion         = safeToVariant(d->albumDB->getSetting("DBVersion"));
    d->currentRequiredVersion = safeToVariant(d->albumDB->getSetting("DBVersionRequired"));
}

void SchemaUpdater::setObserver(InitializationObserver* const observer)
{
    d->observer = observer;
}

bool SchemaUpdater::startUpdates()
{
    if (!d->parameters.isSQLite())
    {
        // Do we have sufficient privileges
        QStringList insufficientRights;
        DatabasePrivilegesChecker checker(d->parameters);

        if (!checker.checkPrivileges(insufficientRights))
        {
            kError() << "Insufficient rights on databse.";
            QString errorMsg = i18n(
                                "You have insufficient privileges on the database.\n"
                                "Following privileges are not assigned to you:\n %1"
                                "\nCheck your privileges on the database and restart digiKam.",
                                insufficientRights.join(",\n")
                            );

            d->lastErrorMessage=errorMsg;

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }
    }

    // First step: do we have an empty database?
    QStringList tables = d->backend->tables();

    if (tables.contains("Albums", Qt::CaseInsensitive))
    {
        // Find out schema version of db file
        readVersionSettings();
        kDebug() << "Have a database structure version " << d->currentVersion.toInt();

        // We absolutely require the DBVersion setting
        if (!d->currentVersion.isValid())
        {
            // Something is damaged. Give up.
            kError() << "DBVersion not available! Giving up schema upgrading.";
            QString errorMsg = i18n(
                                   "The database is not valid: "
                                   "the \"DBVersion\" setting does not exist. "
                                   "The current database schema version cannot be verified. "
                                   "Try to start with an empty database. "
                               );
            d->lastErrorMessage=errorMsg;

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        // current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.
        if (d->currentVersion.toInt() > schemaVersion())
        {
            // trying to open a database with a more advanced than this SchemaUpdater supports
            if (d->currentRequiredVersion.isValid() && d->currentRequiredVersion.toInt() <= schemaVersion())
            {
                // version required may be less than current version
                return true;
            }
            else
            {
                QString errorMsg = i18n("The database has been used with a more recent version of digiKam "
                                        "and has been updated to a database schema which cannot be used with this version. "
                                        "(This means this digiKam version is too old, or the database format is too recent.) "
                                        "Please use the more recent version of digiKam that you used before. ");
                d->lastErrorMessage=errorMsg;

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
        // Version 6 brought new tables for history and ImageTagProperties, with version 2.0
        // Version 7 brought the VideoMetadata table with 3.0

        if (d->parameters.isSQLite())
        {
            QFileInfo currentDBFile(d->parameters.databaseName);
            QFileInfo digikam3DB(currentDBFile.dir(), "digikam3.db");
            QFileInfo digikamDB(currentDBFile.dir(),  "digikam.db");

            if (digikam3DB.exists())
            {
                if (!copyV3toV4(digikam3DB.filePath(), currentDBFile.filePath()))
                {
                    return false;
                }

                // d->currentVersion is now 4;
                return makeUpdates();
            }
            else if (digikamDB.exists())
            {
                if (!updateV2toV4(digikamDB.path()))
                {
                    return false;
                }

                // d->currentVersion is now 4;
                return makeUpdates();
            }

            // no else, fall through!
        }

        // No legacy handling: start with a fresh db
        if (!createDatabase() || !createFilterSettings())
        {
            QString errorMsg   = i18n("Failed to create tables in database.\n ") + d->backend->lastError();
            d->lastErrorMessage = errorMsg;

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

bool SchemaUpdater::beginWrapSchemaUpdateStep()
{
    if (!d->backend->beginTransaction())
    {
        QFileInfo currentDBFile(d->parameters.databaseName);
        QString errorMsg = i18n("Failed to open a database transaction on your database file \"%1\". "
                                "This is unusual. Please check that you can access the file and no "
                                "other process has currently locked the file. "
                                "If the problem persists you can get help from the digikam-devel@kde.org "
                                "mailing list. As well, please have a look at what digiKam prints on the console. ",
                                currentDBFile.filePath());
        d->observer->error(errorMsg);
        d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        return false;
    }

    return true;
}

bool SchemaUpdater::endWrapSchemaUpdateStep(bool stepOperationSuccess, const QString& errorMsg)
{
    if (!stepOperationSuccess)
    {
        d->backend->rollbackTransaction();

        if (d->observer)
        {
            // error or cancelled?
            if (!d->observer->continueQuery())
            {
                kDebug() << "Schema update cancelled by user";
            }
            else if (!d->setError)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }
        }

        return false;
    }

    kDebug() << "Success updating to v5";
    d->backend->commitTransaction();
    return true;
}

bool SchemaUpdater::makeUpdates()
{
    kDebug() << "makeUpdates " << d->currentVersion.toInt() << " to " << schemaVersion();

    if (d->currentVersion.toInt() < schemaVersion())
    {
        if (d->currentVersion.toInt() < 5)
        {
            if (!beginWrapSchemaUpdateStep())
            {
                return false;
            }

            // v4 was always SQLite
            QFileInfo currentDBFile(d->parameters.databaseName);
            QString errorMsg = i18n("The schema updating process from version 4 to 6 failed, "
                                    "caused by an error that we did not expect. "
                                    "You can try to discard your old database and start with an empty one. "
                                    "(In this case, please move the database files "
                                    "\"%1\" and \"%2\" from the directory \"%3\"). "
                                    "More probably you will want to report this error to the digikam-devel@kde.org "
                                    "mailing list. As well, please have a look at what digiKam prints on the console. ",
                                    QString("digikam3.db"), QString("digikam4.db"), currentDBFile.dir().path());

            if (!endWrapSchemaUpdateStep(updateV4toV7(), errorMsg))
            {
                return false;
            }

            kDebug() << "Success updating v4 to v6";

            // Still set these even in >= 1.4 because 0.10 - 1.3 may want to apply the updates if not set
            setLegacySettingEntries();
        }

        // Incremental updates, starting from version 5
        for (int v = d->currentVersion.toInt(); v < schemaVersion(); v++)
        {
            int targetVersion = v + 1;

            if (!beginWrapSchemaUpdateStep())
            {
                return false;
            }

            QString errorMsg = i18n("Failed to update the database schema from version %1 to version %2. "
                                    "Please read the error messages printed on the console and "
                                    "report this error as a bug at bugs.kde.org. ",
                                    d->currentVersion.toInt(), targetVersion);

            if (!endWrapSchemaUpdateStep(updateToVersion(targetVersion), errorMsg))
            {
                return false;
            }

            kDebug() << "Success updating to v" << d->currentVersion;
        }

        // add future updates here
    }

    return true;
}

void SchemaUpdater::defaultFilterSettings(QStringList& defaultImageFilter, QStringList& defaultVideoFilter,
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

    defaultVideoFilter << "mpeg" << "mpg" << "mpo" << "mpe"                              // MPEG
                       << "avi" << "divx"                                                // RIFF
                       << "wmv" << "wmf" << "asf"                                        // ASF
                       << "mp4" << "3gp" << "mov"  << "3g2" << "m4v" << "m2v"            // QuickTime
                       << "mkv" << "webm";                                               // Matroska

    defaultAudioFilter << "ogg" << "mp3" << "wma" << "wav";
}

bool SchemaUpdater::createFilterSettings()
{
    QStringList defaultImageFilter, defaultVideoFilter, defaultAudioFilter;
    defaultFilterSettings(defaultImageFilter, defaultVideoFilter, defaultAudioFilter);

    d->albumDB->setFilterSettings(defaultImageFilter, defaultVideoFilter, defaultAudioFilter);
    d->albumDB->setSetting("FilterSettingsVersion", QString::number(filterSettingsVersion()));
    d->albumDB->setSetting("DcrawFilterSettingsVersion", QString::number(KDcrawIface::KDcraw::rawFilesVersion()));

    return true;
}

bool SchemaUpdater::updateFilterSettings()
{
    QString filterVersion = d->albumDB->getSetting("FilterSettingsVersion");
    QString dcrawFilterVersion = d->albumDB->getSetting("DcrawFilterSettingsVersion");

    if (filterVersion.toInt() < filterSettingsVersion() ||
        dcrawFilterVersion.toInt() < KDcrawIface::KDcraw::rawFilesVersion())
    {
        createFilterSettings();
    }

    return true;
}

bool SchemaUpdater::createDatabase()
{
    if ( createTables() && createIndices() && createTriggers())
    {
        setLegacySettingEntries();

        d->currentVersion = schemaVersion();

        // if we start with the V2 hash, version 6 is required
        d->albumDB->setUniqueHashVersion(uniqueHashVersion());
        d->currentRequiredVersion = schemaVersion();
/*
        // Digikam for database version 5 can work with version 6, though not using the new features
        d->currentRequiredVersion = 5;
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
    return d->backend->execDBAction(d->backend->getDBAction("CreateDB"));
}

bool SchemaUpdater::createIndices()
{
    // TODO: see which more indices are needed
    // create indices
    return d->backend->execDBAction(d->backend->getDBAction("CreateIndices"));
}

bool SchemaUpdater::createTriggers()
{
    return d->backend->execDBAction(d->backend->getDBAction(QString("CreateTriggers")));
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

        if (d->observer)
        {
            d->observer->connectCollectionScanner(&scanner);
            scanner.setObserver(d->observer);
        }

        scanner.completeScan();

        // earlier digikam does not know about the hash
        if (d->currentRequiredVersion.toInt() < 6)
        {
            d->currentRequiredVersion = 6;
            setVersionSettings();
        }
    }
    return true;
}

bool SchemaUpdater::performUpdateToVersion(const QString& actionName, int newVersion, int newRequiredVersion)
{
    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->moreSchemaUpdateSteps(1);
    }

    DatabaseAction updateAction = d->backend->getDBAction(actionName);

    if (updateAction.name.isNull())
    {
        QString errorMsg = i18n("The database update action cannot be found. Please ensure that "
                                "the dbconfig.xml file of the current version of digiKam is installed "
                                "at the correct place. ");
    }

    if (!d->backend->execDBAction(updateAction))
    {
        kError() << "Schema update to V" << newVersion << "failed!";
        // resort to default error message, set above
        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Updated schema to version %1.", newVersion));
    }

    d->currentVersion = newVersion;
    // Digikam for database version 5 can work with version 6, though not using the new features
    // Note: We do not upgrade the uniqueHash
    d->currentRequiredVersion = newRequiredVersion;
    return true;
}


bool SchemaUpdater::updateToVersion(int targetVersion)
{
    if (d->currentVersion != targetVersion-1)
    {
        kError() << "updateToVersion performs only incremental updates. Called to update from"
                 << d->currentVersion << "to" << targetVersion << ", aborting.";
        return false;
    }

    switch (targetVersion)
    {
        case 6:
            // Digikam for database version 5 can work with version 6, though not using the new features
            // Note: We do not upgrade the uniqueHash
            return performUpdateToVersion("UpdateSchemaFromV5ToV6", 6, 5);
        case 7:
            // Digikam for database version 5 and 6 can work with version 7, though not using the support for video files.
            return performUpdateToVersion("UpdateSchemaFromV6ToV7", 7, 5);
        // NOTE: If you add a new update step, please check the d->currentVersion at the bottom of updateV4toV7
            // If the update already comes with createTables, createTriggers, we dont need the extra update here
        default:
            kError() << "Unsupported update to version" << targetVersion;
            return false;
    }
}

bool SchemaUpdater::copyV3toV4(const QString& digikam3DBPath, const QString& currentDBPath)
{
    if (d->observer)
    {
        d->observer->moreSchemaUpdateSteps(2);
    }

    d->backend->close();

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
        d->lastErrorMessage=errorMsg;
        d->setError = true;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (d->observer)
    {
        d->observer->schemaUpdateProgress(i18n("Copied database file"));
    }

    if (!d->backend->open(d->parameters))
    {
        QString errorMsg = i18n("The old database file (\"%1\") has been copied "
                                "to the new location (\"%2\") but it cannot be opened. "
                                "Please delete both files and try again, "
                                "starting with an empty database. ",
                                digikam3DBPath, currentDBPath);

        d->lastErrorMessage = errorMsg;
        d->setError         = true;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (d->observer)
    {
        d->observer->schemaUpdateProgress(i18n("Opened new database file"));
    }

    d->currentVersion = 4;
    return true;
}

bool SchemaUpdater::updateV2toV4(const QString& sqlite2DBPath)
{
    if (d->observer)
    {
        d->observer->moreSchemaUpdateSteps(1);
    }

    bool ret = false;

#ifdef HAVE_SQLITE2
    if (upgradeDB_Sqlite2ToSqlite3(d->albumDB, d->backend, sqlite2DBPath))
    {
        d->currentVersion = 4;
        ret = true;
    }
    else
#endif
    {
        QString errorMsg    = i18n("Could not update from the old SQLite2 file (\"%1\"). "
                                   "Please delete this file and try again, "
                                   "starting with an empty database. ", sqlite2DBPath);
        d->lastErrorMessage = errorMsg;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }
    }

    if (d->observer)
    {
        d->observer->schemaUpdateProgress(i18n("Updated from 0.7 database"));
    }

    return ret;
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

bool SchemaUpdater::updateV4toV7()
{
    kDebug() << "updateV4toV7";

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->moreSchemaUpdateSteps(11);
    }

    // This update was introduced from digikam version 0.9 to digikam 0.10
    // We operator on an SQLite3 database under a transaction (which will be rolled back on error)

    // --- Make space for new tables ---
    if (!d->backend->execSql(QString("ALTER TABLE Albums RENAME TO AlbumsV3;")))
    {
        return false;
    }

    if (!d->backend->execSql(QString("ALTER TABLE Images RENAME TO ImagesV3;")))
    {
        return false;
    }

    if (!d->backend->execSql(QString("ALTER TABLE Searches RENAME TO SearchesV3;")))
    {
        return false;
    }

    kDebug() << "Moved tables";
    // --- Drop some triggers and indices ---

    // Don't check for errors here. The "IF EXISTS" clauses seem not supported in SQLite
    d->backend->execSql(QString("DROP TRIGGER delete_album;"));
    d->backend->execSql(QString("DROP TRIGGER delete_image;"));
    d->backend->execSql(QString("DROP TRIGGER delete_tag;"));
    d->backend->execSql(QString("DROP TRIGGER insert_tagstree;"));
    d->backend->execSql(QString("DROP TRIGGER delete_tagstree;"));
    d->backend->execSql(QString("DROP TRIGGER move_tagstree;"));
    d->backend->execSql(QString("DROP INDEX dir_index;"));
    d->backend->execSql(QString("DROP INDEX tag_index;"));

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Prepared table creation"));
    }

    kDebug() << "Dropped triggers";

    // --- Create new tables ---

    if (!createTables() || !createIndices())
    {
        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Created tables"));
    }

    // --- Populate AlbumRoots (from config) ---

    KSharedConfigPtr config  = KGlobal::config();
    KConfigGroup group       = config->group("Album Settings");
    QString albumLibraryPath = group.readEntry("Album Path", QString());

    if (albumLibraryPath.isEmpty())
    {
        kError() << "Album library path from config file is empty. Aborting update.";
        QString errorMsg    = i18n("No album library path has been found in the configuration file. "
                                   "Giving up the schema updating process. "
                                   "Please try with an empty database, or repair your configuration.");
        d->lastErrorMessage = errorMsg;
        d->setError         = true;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    CollectionLocation location =
        CollectionManager::instance()->addLocation(KUrl::fromPath(albumLibraryPath));

    if (location.isNull())
    {
        kError() << "Failure to create a collection location. Aborting update.";
        QString errorMsg    = i18n("There was an error associating your albumLibraryPath (\"%1\") "
                                   "with a storage volume of your system. "
                                   "This problem may indicate that there is a problem with your installation. "
                                   "If you are working on Linux, check that HAL is installed and running. "
                                   "In any case, you can seek advice from the digikam-devel@kde.org mailing list. "
                                   "The database updating process will now be aborted because we do not want "
                                   "to create a new database based on false assumptions from a broken installation.",
                                   albumLibraryPath);
        d->lastErrorMessage = errorMsg;
        d->setError         = true;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Configured one album root"));
    }

    kDebug() << "Inserted album root";

    // --- With the album root, populate albums ---

    if (!d->backend->execSql(QString(
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

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported albums"));
    }

    kDebug() << "Populated albums";

    // --- Add images ---

    if (!d->backend->execSql(QString(
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

    if (!d->access->backend()->execSql(QString(
                                          "REPLACE INTO ImageInformation (imageId) SELECT id FROM Images;"))
       )
    {
        return false;
    }

    // remove orphan images that would not be removed by CollectionScanner
    d->backend->execSql(QString("DELETE FROM Images WHERE album NOT IN (SELECT id FROM Albums);"));

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported images information"));
    }

    kDebug() << "Populated Images";

    // --- Port searches ---

    if (!d->backend->execSql(QString(
                                "REPLACE INTO Searches "
                                " (id, type, name, query) "
                                "SELECT id, ?, name, url"
                                " FROM SearchesV3;"),
                            DatabaseSearch::LegacyUrlSearch)
       )
    {
        return false;
    }

    SearchInfo::List sList = d->albumDB->scanSearches();

    for (SearchInfo::List::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
    {
        KUrl url((*it).query);

        ImageQueryBuilder builder;
        QString query = builder.convertFromUrlToXml(url);
        QString name  = (*it).name;

        if (name == i18n("Last Search"))
        {
            name = i18n("Last Search (0.9)");
        }

        if (url.queryItem("type") == QString("datesearch"))
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::TimeLineSearch, name, query);
        }
        else if (url.queryItem("1.key") == "keyword")
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::KeywordSearch, name, query);
        }
        else
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::AdvancedSearch, name, query);
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

    d->albumDB->setUserFilterSettings(configImageFilter.toList(), configVideoFilter.toList(), configAudioFilter.toList());
    kDebug() << "Set initial filter settings with user settings" << configImageFilter;

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Initialized and imported file suffix filter"));
    }

    // --- do a full scan ---

    CollectionScanner scanner;

    if (d->observer)
    {
        d->observer->connectCollectionScanner(&scanner);
        scanner.setObserver(d->observer);
    }

    scanner.completeScan();

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Did the initial full scan"));
    }

    // --- Port date, comment and rating (_after_ the scan) ---

    // Port ImagesV3.date -> ImageInformation.creationDate
    if (!d->backend->execSql(QString(
                                "UPDATE ImageInformation SET "
                                " creationDate=(SELECT datetime FROM ImagesV3 WHERE ImagesV3.id=ImageInformation.imageid) "
                                "WHERE imageid IN (SELECT id FROM ImagesV3);")
                           )
       )
    {
        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported creation dates"));
    }

    // Port ImagesV3.comment to ImageComments

    // An author of NULL will inhibt the UNIQUE restriction to take effect (but #189080). Work around.
    d->backend->execSql(QString(
                           "DELETE FROM ImageComments WHERE "
                           "type=? AND language=? AND author IS NULL "
                           "AND imageid IN ( SELECT id FROM ImagesV3 ); "),
                       (int)DatabaseComment::Comment, QString("x-default"));

    if (!d->backend->execSql(QString(
                                "REPLACE INTO ImageComments "
                                " (imageid, type, language, comment) "
                                "SELECT id, ?, ?, caption FROM ImagesV3;"
                            ),
                            (int)DatabaseComment::Comment, QString("x-default"))
       )
    {
        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported comments"));
    }

    // Port rating storage in ImageProperties to ImageInformation
    if (!d->backend->execSql(QString(
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

    d->backend->execSql(QString("DELETE FROM ImageProperties WHERE property=?;"), QString("Rating"));
    d->backend->execSql(QString("UPDATE ImageInformation SET rating=0 WHERE rating<0;"));

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported ratings"));
    }

    // --- Drop old tables ---

    d->backend->execSql(QString("DROP TABLE ImagesV3;"));
    d->backend->execSql(QString("DROP TABLE AlbumsV3;"));
    d->backend->execSql(QString("DROP TABLE SearchesV3;"));

    if (d->observer)
    {
        d->observer->schemaUpdateProgress(i18n("Dropped v3 tables"));
    }

    d->currentRequiredVersion = 5;
    d->currentVersion = 7;
    kDebug() << "Returning true from updating to 5";
    return true;
}

void SchemaUpdater::setLegacySettingEntries()
{
    d->albumDB->setSetting("preAlpha010Update1", "true");
    d->albumDB->setSetting("preAlpha010Update2", "true");
    d->albumDB->setSetting("preAlpha010Update3", "true");
    d->albumDB->setSetting("beta010Update1",     "true");
    d->albumDB->setSetting("beta010Update2",     "true");
}

// ---------- Legacy code ------------

void SchemaUpdater::preAlpha010Update1()
{
    QString hasUpdate = d->albumDB->getSetting("preAlpha010Update1");

    if (!hasUpdate.isNull())
    {
        return;
    }

    if (!d->backend->execSql(QString("ALTER TABLE Searches RENAME TO SearchesV3;")))
    {
        return;
    }

    if ( !d->backend->execSql(
             QString( "CREATE TABLE IF NOT EXISTS Searches  \n"
                      " (id INTEGER PRIMARY KEY, \n"
                      "  type INTEGER, \n"
                      "  name TEXT NOT NULL, \n"
                      "  query TEXT NOT NULL);" ) ))
    {
        return;
    }

    if (!d->backend->execSql(QString(
                                "REPLACE INTO Searches "
                                " (id, type, name, query) "
                                "SELECT id, ?, name, url"
                                " FROM SearchesV3;"),
                            DatabaseSearch::LegacyUrlSearch)
       )
    {
        return;
    }

    SearchInfo::List sList = d->albumDB->scanSearches();

    for (SearchInfo::List::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
    {
        KUrl url((*it).query);

        ImageQueryBuilder builder;
        QString query = builder.convertFromUrlToXml(url);

        if (url.queryItem("type") == QString("datesearch"))
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::TimeLineSearch, (*it).name, query);
        }
        else if (url.queryItem("1.key") == "keyword")
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::KeywordSearch, (*it).name, query);
        }
        else
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::AdvancedSearch, (*it).name, query);
        }
    }

    d->backend->execSql(QString("DROP TABLE SearchesV3;"));

    d->albumDB->setSetting("preAlpha010Update1", "true");
}

void SchemaUpdater::preAlpha010Update2()
{
    QString hasUpdate = d->albumDB->getSetting("preAlpha010Update2");

    if (!hasUpdate.isNull())
    {
        return;
    }

    if (!d->backend->execSql(QString("ALTER TABLE ImagePositions RENAME TO ImagePositionsTemp;")))
    {
        return;
    }

    if (!d->backend->execSql(QString("ALTER TABLE ImageMetadata RENAME TO ImageMetadataTemp;")))
    {
        return;
    }

    d->backend->execSql(
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

    d->backend->execSql(QString(
                           "REPLACE INTO ImagePositions "
                           " (imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                           "  altitude, orientation, tilt, roll, accuracy, description) "
                           "SELECT imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                           "  altitude, orientation, tilt, roll, 0, description "
                           " FROM ImagePositionsTemp;"));

    d->backend->execSql(
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

    d->backend->execSql( QString("INSERT INTO ImageMetadata "
                                " (imageid, make, model, lens, aperture, focalLength, focalLength35, "
                                "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                                "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) "
                                "SELECT imageid, make, model, NULL, aperture, focalLength, focalLength35, "
                                "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                                "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory "
                                "FROM ImageMetadataTemp;"));

    d->backend->execSql(QString("DROP TABLE ImagePositionsTemp;"));
    d->backend->execSql(QString("DROP TABLE ImageMetadataTemp;"));

    d->albumDB->setSetting("preAlpha010Update2", "true");
}

void SchemaUpdater::preAlpha010Update3()
{
    QString hasUpdate = d->albumDB->getSetting("preAlpha010Update3");

    if (!hasUpdate.isNull())
    {
        return;
    }

    d->backend->execSql(QString("DROP TABLE ImageCopyright;"));
    d->backend->execSql(
        QString("CREATE TABLE ImageCopyright\n"
                " (imageid INTEGER,\n"
                "  property TEXT,\n"
                "  value TEXT,\n"
                "  extraValue TEXT,\n"
                "  UNIQUE(imageid, property, value, extraValue));")
    );

    d->albumDB->setSetting("preAlpha010Update3", "true");
}

void SchemaUpdater::beta010Update1()
{
    QString hasUpdate = d->albumDB->getSetting("beta010Update1");

    if (!hasUpdate.isNull())
    {
        return;
    }

    // if Image has been deleted
    d->backend->execSql("DROP TRIGGER delete_image;");
    d->backend->execSql(
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


    d->albumDB->setSetting("beta010Update1", "true");
}

void SchemaUpdater::beta010Update2()
{
    QString hasUpdate = d->albumDB->getSetting("beta010Update2");

    if (!hasUpdate.isNull())
    {
        return;
    }

    // force rescan and creation of ImageInformation entry for videos and audio
    d->backend->execSql("DELETE FROM Images WHERE category=2 OR category=3;");

    d->albumDB->setSetting("beta010Update2", "true");
}

bool SchemaUpdater::createTablesV3()
{
    if (!d->backend->execSql( QString("CREATE TABLE Albums\n"
                                     " (id INTEGER PRIMARY KEY,\n"
                                     "  url TEXT NOT NULL UNIQUE,\n"
                                     "  date DATE NOT NULL,\n"
                                     "  caption TEXT,\n"
                                     "  collection TEXT,\n"
                                     "  icon INTEGER);") ))
    {
        return false;
    }

    if (!d->backend->execSql( QString("CREATE TABLE Tags\n"
                                     " (id INTEGER PRIMARY KEY,\n"
                                     "  pid INTEGER,\n"
                                     "  name TEXT NOT NULL,\n"
                                     "  icon INTEGER,\n"
                                     "  iconkde TEXT,\n"
                                     "  UNIQUE (name, pid));") ))
    {
        return false;
    }

    if (!d->backend->execSql( QString("CREATE TABLE TagsTree\n"
                                     " (id INTEGER NOT NULL,\n"
                                     "  pid INTEGER NOT NULL,\n"
                                     "  UNIQUE (id, pid));") ))
    {
        return false;
    }

    if (!d->backend->execSql( QString("CREATE TABLE Images\n"
                                     " (id INTEGER PRIMARY KEY,\n"
                                     "  name TEXT NOT NULL,\n"
                                     "  dirid INTEGER NOT NULL,\n"
                                     "  caption TEXT,\n"
                                     "  datetime DATETIME,\n"
                                     "  UNIQUE (name, dirid));") ))
    {
        return false;
    }


    if (!d->backend->execSql( QString("CREATE TABLE ImageTags\n"
                                     " (imageid INTEGER NOT NULL,\n"
                                     "  tagid INTEGER NOT NULL,\n"
                                     "  UNIQUE (imageid, tagid));") ))
    {
        return false;
    }

    if (!d->backend->execSql( QString("CREATE TABLE ImageProperties\n"
                                     " (imageid  INTEGER NOT NULL,\n"
                                     "  property TEXT    NOT NULL,\n"
                                     "  value    TEXT    NOT NULL,\n"
                                     "  UNIQUE (imageid, property));") ))
    {
        return false;
    }

    if ( !d->backend->execSql( QString( "CREATE TABLE Searches  \n"
                                       " (id INTEGER PRIMARY KEY, \n"
                                       "  name TEXT NOT NULL UNIQUE, \n"
                                       "  url  TEXT NOT NULL);" ) ) )
    {
        return false;
    }

    if (!d->backend->execSql( QString("CREATE TABLE Settings         \n"
                                     "(keyword TEXT NOT NULL UNIQUE,\n"
                                     " value TEXT);") ))
    {
        return false;
    }

    // TODO: see which more indices are needed
    // create indices
    d->backend->execSql("CREATE INDEX dir_index ON Images    (dirid);");
    d->backend->execSql("CREATE INDEX tag_index ON ImageTags (tagid);");

    // create triggers

    // trigger: delete from Images/ImageTags/ImageProperties
    // if Album has been deleted
    d->backend->execSql("CREATE TRIGGER delete_album DELETE ON Albums\n"
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
    d->backend->execSql("CREATE TRIGGER delete_image DELETE ON Images\n"
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
    d->backend->execSql("CREATE TRIGGER delete_tag DELETE ON Tags\n"
                       "BEGIN\n"
                       "  DELETE FROM ImageTags WHERE tagid=OLD.id;\n"
                       "END;");

    // trigger: insert into TagsTree if Tag has been added
    d->backend->execSql("CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags\n"
                       "BEGIN\n"
                       "  INSERT INTO TagsTree\n"
                       "    SELECT NEW.id, NEW.pid\n"
                       "    UNION\n"
                       "    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;\n"
                       "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    d->backend->execSql("CREATE TRIGGER delete_tagstree DELETE ON Tags\n"
                       "BEGIN\n"
                       " DELETE FROM Tags\n"
                       "   WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
                       " DELETE FROM TagsTree\n"
                       "   WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
                       " DELETE FROM TagsTree\n"
                       "    WHERE id=OLD.id;\n"
                       "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    d->backend->execSql("CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags\n"
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
