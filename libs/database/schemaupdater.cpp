/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <QFileInfo>
#include <QFile>
#include <QDir>

// KDE includes.

#include <kdebug.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "databasebackend.h"
#include "albumdb.h"
#include "databasetransaction.h"
#include "upgradedb_sqlite2tosqlite3.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionscanner.h"
#include "imagequerybuilder.h"
#include "initializationobserver.h"

namespace Digikam
{

int SchemaUpdater::schemaVersion()
{
    return 5;
}

int SchemaUpdater::filterSettingsVersion()
{
    return 2;
}

SchemaUpdater::SchemaUpdater(DatabaseAccess *access)
{
    m_access         = access;
    m_currentVersion = 0;
    m_observer       = 0;
    m_setError       = false;
}

bool SchemaUpdater::update()
{
    kDebug(50003) << "SchemaUpdater update" << endl;
    bool success = startUpdates();
    // even on failure, try to set current version - it may have incremented
    m_access->db()->setSetting("DBVersion",QString::number(m_currentVersion));
    if (!success)
        return false;
    updateFilterSettings();

    if (m_observer)
        m_observer->finishedSchemaUpdate(InitializationObserver::UpdateSuccess);

    return success;
}

void SchemaUpdater::setObserver(InitializationObserver *observer)
{
    m_observer = observer;
}

bool SchemaUpdater::startUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = m_access->backend()->tables();

    if (tables.contains("Albums"))
    {
        // Find out schema version of db file
        QString version = m_access->db()->getSetting("DBVersion");
        QString versionRequired = m_access->db()->getSetting("DBVersionRequired");
        kDebug(50003) << "Have a database structure version " << version << endl;

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
            // trying to open a database with a more advanced than this SchemaUpdater supports
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
        kDebug(50003) << "No database file available" << endl;
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

        DatabaseParameters parameters = m_access->parameters();
        if (parameters.isSQLite())
        {
            QFileInfo currentDBFile(parameters.databaseName);
            QFileInfo digikam3DB(currentDBFile.dir(), "digikam3.db");
            QFileInfo digikamDB(currentDBFile.dir(), "digikam.db");

            if (digikam3DB.exists())
            {
                if (!copyV3toV4(digikam3DB.filePath(), currentDBFile.filePath()))
                    return false;

                // m_currentVersion is now 4;
                return makeUpdates();
            }
            else if (digikamDB.exists())
            {
                if (!updateV2toV4(digikamDB.path()))
                    return false;

                // m_currentVersion is now 4;
                return makeUpdates();
            }
            // no else, fall through!
        }

        // No legacy handling: start with a fresh db
        if (!createDatabase() || !createFilterSettings())
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

bool SchemaUpdater::makeUpdates()
{
    kDebug(50003) << "makeUpdates " << m_currentVersion << " to " << schemaVersion() << endl;
    //DatabaseTransaction transaction(m_access);
    if (m_currentVersion < schemaVersion())
    {
        if (m_currentVersion < 5)
        {
            if (!m_access->backend()->beginTransaction())
            {
                QFileInfo currentDBFile(m_access->parameters().databaseName);
                QString errorMsg = i18n("Failed to open a database transaction on your database file \"%1\". "
                                        "This is unusual. Please check that you can access the file and no "
                                        "other process has currently locked the file. "
                                        "If the problem persists you can get help from the digikam-devel@kde.org "
                                        "mailing list. As well, please have a look at what digiKam prints on the console. ",
                                        currentDBFile.filePath());
                m_observer->error(errorMsg);
                m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }
            if (!updateV4toV5())
            {
                m_access->backend()->rollbackTransaction();
                if (m_observer && !m_setError)
                {
                    QFileInfo currentDBFile(m_access->parameters().databaseName);
                    QString errorMsg = i18n("The schema updating process from version 4 to 5 failed, "
                                            "caused by an error that we did not expect. "
                                            "You can try to discard your old database and start with an empty one. "
                                            "(In this case, please move the database files "
                                            "\"%1\" and \"%2\" from the directory \"%3\"). "
                                            "More probably you will want to report this error to the digikam-devel@kde.org "
                                            "mailing list. As well, please have a look at what digiKam prints on the console. ",
                                            QString("digikam3.db"), QString("digikam4.db"), currentDBFile.dir().path());
                    m_observer->error(errorMsg);
                    m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
                }
                return false;
            }
            kDebug(50003) << "Success updating to v5" << endl;
            m_access->backend()->commitTransaction();
            // REMOVE BEFORE FINAL VERSION
            m_access->db()->setSetting("preAlpha010Update1", "true");
            m_access->db()->setSetting("preAlpha010Update2", "true");
            m_access->db()->setSetting("preAlpha010Update3", "true");
            // END REMOVE
            // REMOVE BEFORE NEXT SCHEMA UPDATE
            m_access->db()->setSetting("beta010Update1", "true");
            m_access->db()->setSetting("beta010Update2", "true");
            // END REMOVE
        }
        // add future updates here
    }
    else
    {
        // REMOVE BEFORE FINAL VERSION
        preAlpha010Update1();
        preAlpha010Update2();
        preAlpha010Update3();
        // END REMOVE
        // REMOVE BEFORE NEXT SCHEMA UPDATE
        beta010Update1();
        beta010Update2();
        // END REMOVE
    }
    return true;
}

void SchemaUpdater::defaultFilterSettings(QStringList &defaultImageFilter,
                                          QStringList &defaultVideoFilter,
                                          QStringList &defaultAudioFilter)
{
    //NOTE for updating:
    //When changing anything here, just increment filterSettingsVersion() so that the changes take effect

    defaultImageFilter << "jpg" << "jpeg" << "jpe"                    // JPEG
                       << "jp2" << "j2k" << "jpx"  << "jpc" << "pgx"  // JPEG-2000
                       << "tif" << "tiff"                             // TIFF
                       << "png"                                       // PNG
                       << "xpm" << "ppm" << "pnm"
                       << "gif" << "bmp" << "xcf" << "pcx";

#if KDCRAW_VERSION < 0x000400
    defaultImageFilter << KDcrawIface::DcrawBinary::rawFilesList();
#else
    defaultImageFilter << KDcrawIface::KDcraw::rawFilesList();
#endif

    defaultVideoFilter << "mpeg" << "mpg" << "mpo" << "mpe"     // MPEG
                       << "avi"  << "mov" << "wmf" << "asf" << "mp4" << "3gp" << "wmv";

    defaultAudioFilter << "ogg" << "mp3" << "wma" << "wav";
}

bool SchemaUpdater::createFilterSettings()
{
    QStringList defaultImageFilter, defaultVideoFilter, defaultAudioFilter;
    defaultFilterSettings(defaultImageFilter, defaultVideoFilter, defaultAudioFilter);

    m_access->db()->setFilterSettings(defaultImageFilter, defaultVideoFilter, defaultAudioFilter);
    m_access->db()->setSetting("FilterSettingsVersion", QString::number(filterSettingsVersion()));
#if KDCRAW_VERSION < 0x000400
    m_access->db()->setSetting("DcrawFilterSettingsVersion", QString::number(KDcrawIface::DcrawBinary::rawFilesVersion()));
#else
    m_access->db()->setSetting("DcrawFilterSettingsVersion", QString::number(KDcrawIface::KDcraw::rawFilesVersion()));
#endif

    return true;
}

bool SchemaUpdater::updateFilterSettings()
{
    QString filterVersion = m_access->db()->getSetting("FilterSettingsVersion");
    QString dcrawFilterVersion = m_access->db()->getSetting("DcrawFilterSettingsVersion");

    if (
         filterVersion.toInt() < filterSettingsVersion() ||
#if KDCRAW_VERSION < 0x000400
         dcrawFilterVersion.toInt() < KDcrawIface::DcrawBinary::rawFilesVersion()
#else
         dcrawFilterVersion.toInt() < KDcrawIface::KDcraw::rawFilesVersion()
#endif
       )
    {
        createFilterSettings();
    }
    return true;
}

bool SchemaUpdater::createDatabase()
{
    if ( createTablesV5()
         && createIndicesV5()
         && createTriggersV5())
    {
        // REMOVE BEFORE ALPHA VERSION
        m_access->db()->setSetting("preAlpha010Update1", "true");
        m_access->db()->setSetting("preAlpha010Update2", "true");
        m_access->db()->setSetting("preAlpha010Update3", "true");
        // END REMOVE
        // REMOVE BEFORE NEXT SCHEMA UPDATE
        m_access->db()->setSetting("beta010Update1", "true");
        m_access->db()->setSetting("beta010Update2", "true");
        // END REMOVE
        m_currentVersion = 5;
        return true;
    }
    else
        return false;
}

bool SchemaUpdater::createTablesV5()
{
    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE AlbumRoots\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  label TEXT,\n"
                            "  status INTEGER NOT NULL,\n"
                            "  type INTEGER NOT NULL,\n"
                            "  identifier TEXT,\n"
                            "  specificPath TEXT,\n"
                            "  UNIQUE(identifier, specificPath));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE Albums\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  albumRoot INTEGER NOT NULL,\n"
                            "  relativePath TEXT NOT NULL,\n"
                            "  date DATE,\n"
                            "  caption TEXT,\n"
                            "  collection TEXT,\n"
                            "  icon INTEGER,\n"
                            "  UNIQUE(albumRoot, relativePath));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE Images\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  album INTEGER,\n" // no constraints, for temporary orphans
                            "  name TEXT NOT NULL,\n"
                            "  status INTEGER NOT NULL,\n"
                            "  category INTEGER NOT NULL,\n"
                            "  modificationDate DATETIME,\n"
                            "  fileSize INTEGER,\n"
                            "  uniqueHash TEXT,\n"
                            "  UNIQUE (album, name));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageHaarMatrix\n"
                            " (imageid INTEGER PRIMARY KEY,\n"
                            "  modificationDate DATETIME,\n"
                            "  uniqueHash TEXT,\n"
                            "  matrix BLOB);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageInformation\n"
                            " (imageid INTEGER PRIMARY KEY,\n"
                            "  rating INTEGER,\n"
                            "  creationDate DATETIME,\n"
                            "  digitizationDate DATETIME,\n"
                            "  orientation INTEGER,\n"
                            "  width INTEGER,\n"
                            "  height INTEGER,\n"
                            "  format TEXT,\n"
                            "  colorDepth INTEGER,\n"
                            "  colorModel INTEGER);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
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
                            "  subjectDistanceCategory INTEGER);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
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
                            "  description TEXT);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageComments\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  imageid INTEGER,\n"
                            "  type INTEGER,\n"
                            "  language TEXT,\n"
                            "  author TEXT,\n"
                            "  date DATETIME,\n"
                            "  comment TEXT,\n"
                            "  UNIQUE(imageid, type, language, author));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageCopyright\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  imageid INTEGER,\n"
                            "  property TEXT,\n"
                            "  value TEXT,\n"
                            "  extraValue TEXT,\n"
                            "  UNIQUE(imageid, property, value, extraValue));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE IF NOT EXISTS Tags\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  pid INTEGER,\n"
                            "  name TEXT NOT NULL,\n"
                            "  icon INTEGER,\n"
                            "  iconkde TEXT,\n"
                            "  UNIQUE (name, pid));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE IF NOT EXISTS TagsTree\n"
                            " (id INTEGER NOT NULL,\n"
                            "  pid INTEGER NOT NULL,\n"
                            "  UNIQUE (id, pid));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE IF NOT EXISTS ImageTags\n"
                            " (imageid INTEGER NOT NULL,\n"
                            "  tagid INTEGER NOT NULL,\n"
                            "  UNIQUE (imageid, tagid));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE IF NOT EXISTS ImageProperties\n"
                            " (imageid  INTEGER NOT NULL,\n"
                            "  property TEXT    NOT NULL,\n"
                            "  value    TEXT    NOT NULL,\n"
                            "  UNIQUE (imageid, property));") ))
    {
        return false;
    }

    if ( !m_access->backend()->execSql(
                   QString( "CREATE TABLE IF NOT EXISTS Searches  \n"
                            " (id INTEGER PRIMARY KEY, \n"
                            "  type INTEGER, \n"
                            "  name TEXT NOT NULL, \n"
                            "  query TEXT NOT NULL);" ) ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE DownloadHistory\n"
                            " (id  INTEGER PRIMARY KEY,\n"
                            "  identifier TEXT,\n"
                            "  filename TEXT,\n"
                            "  filesize INTEGER,\n"
                            "  filedate DATETIME,\n"
                            "  UNIQUE(identifier, filename, filesize, filedate));"
                           ) ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE IF NOT EXISTS Settings         \n"
                            "(keyword TEXT NOT NULL UNIQUE,\n"
                            " value TEXT);") ))
    {
        return false;
    }

    return true;
}

bool SchemaUpdater::createIndicesV5()
{
    // TODO: see which more indices are needed
    // create indices
    m_access->backend()->execSql("CREATE INDEX dir_index  ON Images    (album);");
    m_access->backend()->execSql("CREATE INDEX hash_index ON Images    (uniqueHash);");
    m_access->backend()->execSql("CREATE INDEX tag_index  ON ImageTags (tagid);");

    return true;
}

bool SchemaUpdater::createTriggersV5()
{
    // Triggers for deletion

    // if AlbumRoot has been deleted
    m_access->backend()->execSql("CREATE TRIGGER delete_albumroot DELETE ON AlbumRoots\n"
            "BEGIN\n"
            " DELETE FROM Albums\n"
            "   WHERE Albums.albumRoot = OLD.id;\n"
            "END;");

    // if Album has been deleted
    m_access->backend()->execSql("CREATE TRIGGER delete_album DELETE ON Albums\n"
            "BEGIN\n"
            " DELETE FROM Images\n"
            "   WHERE Images.album = OLD.id;\n"
            "END;");

    // if Image has been deleted
    // NOTE: For update to v6, merge in beta010Update1
    m_access->backend()->execSql(
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

    // trigger: delete from ImageTags if Tag has been deleted
    m_access->backend()->execSql(
            "CREATE TRIGGER delete_tag DELETE ON Tags\n"
            "BEGIN\n"
            "  DELETE FROM ImageTags WHERE tagid=OLD.id;\n"
            "END;");


    // Triggers maintaining the TagTree (which is used when listing images by tags)

    // trigger: insert into TagsTree if Tag has been added
    m_access->backend()->execSql(
            "CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags\n"
            "BEGIN\n"
            "  INSERT INTO TagsTree\n"
            "    SELECT NEW.id, NEW.pid\n"
            "    UNION\n"
            "    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;\n"
            "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    m_access->backend()->execSql(
            "CREATE TRIGGER delete_tagstree DELETE ON Tags\n"
            "BEGIN\n"
            " DELETE FROM Tags\n"
            "   WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
            " DELETE FROM TagsTree\n"
            "   WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
            " DELETE FROM TagsTree\n"
            "    WHERE id=OLD.id;\n"
            "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    m_access->backend()->execSql(
            "CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags\n"
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

bool SchemaUpdater::copyV3toV4(const QString &digikam3DBPath, const QString &currentDBPath)
{
    if (m_observer)
        m_observer->moreSchemaUpdateSteps(2);

    m_access->backend()->close();

    // We cannot use KIO here because KIO only works from the main thread
    QFile oldFile(digikam3DBPath);
    QFile newFile(currentDBPath);
    // QFile won't override. Remove the empty db file created when a non-existent file is opened
    newFile.remove();
    if (!oldFile.copy(currentDBPath))
    {
        QString errorMsg = i18n("Failed to copy the old database file (\"%1\")"
                                "to its new location (\"%2\")."
                                "Error message: \"%3\"."
                                "Please make sure that the file can be copied, "
                                "or delete it.",
                                digikam3DBPath, currentDBPath, oldFile.errorString());
        m_access->setLastError(errorMsg);
        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }
        return false;
    }
    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Copied database file"));

    if (!m_access->backend()->open(m_access->parameters()))
    {
        QString errorMsg = i18n("The old database file (\"%1\") has been copied "
                                "to the new location (\"%2\") but it cannot be opened. "
                                "Please delete both files and try again, "
                                "starting with an empty database. ",
                                digikam3DBPath, currentDBPath);

        m_access->setLastError(errorMsg);
        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }
        return false;
    }
    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Opened new database file"));

    m_currentVersion = 4;
    return true;
}

bool SchemaUpdater::updateV2toV4(const QString &sqlite2DBPath)
{
    if (m_observer)
        m_observer->moreSchemaUpdateSteps(1);

    if (upgradeDB_Sqlite2ToSqlite3(*m_access, sqlite2DBPath))
    {
        m_currentVersion = 4;
        return true;
    }
    else
    {
        QString errorMsg = i18n("Could not update from the old SQLite2 file (\"%1\"). "
                                "Please delete this file and try again, "
                                "starting with an empty database. ", sqlite2DBPath);
        m_access->setLastError(errorMsg);
        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }
        return false;
    }
    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Updated from 0.7 database"));
}

static QStringList cleanUserFilterString(const QString &filterString)
{
    // splits by either ; or space, removes "*.", trims
    QStringList filterList;

    QString wildcard("*.");
    QChar dot('.');

    QChar sep(';');
    int i = filterString.indexOf( sep );
    if ( i == -1 && filterString.indexOf(' ') != -1 )
        sep = QChar(' ');

    QStringList sepList = filterString.split(sep, QString::SkipEmptyParts);
    foreach (const QString &f, sepList)
    {
        if (f.startsWith(wildcard))
            filterList << f.mid(2).trimmed().toLower();
        else
            filterList << f.trimmed().toLower();
    }
    return filterList;
}

bool SchemaUpdater::updateV4toV5()
{
    kDebug(50003) << "updateV4toV5" << endl;
    if (m_observer)
        m_observer->moreSchemaUpdateSteps(11);

    // This update was introduced from digikam version 0.9 to digikam 0.10
    // We operator on an SQLite3 database under a transaction (which will be rolled back on error)

    // --- Make space for new tables ---
    if (!m_access->backend()->execSql(QString("ALTER TABLE Albums RENAME TO AlbumsV3;")))
        return false;

    if (!m_access->backend()->execSql(QString("ALTER TABLE Images RENAME TO ImagesV3;")))
        return false;

    if (!m_access->backend()->execSql(QString("ALTER TABLE Searches RENAME TO SearchesV3;")))
        return false;

    kDebug(50003) << "Moved tables" << endl;
    // --- Drop some triggers and indices ---

    // Don't check for errors here. The "IF EXISTS" clauses seem not supported in SQLite
    m_access->backend()->execSql(QString("DROP TRIGGER delete_album;"));
    m_access->backend()->execSql(QString("DROP TRIGGER delete_image;"));
    m_access->backend()->execSql(QString("DROP TRIGGER delete_tag;"));
    m_access->backend()->execSql(QString("DROP TRIGGER insert_tagstree;"));
    m_access->backend()->execSql(QString("DROP TRIGGER delete_tagstree;"));
    m_access->backend()->execSql(QString("DROP TRIGGER move_tagstree;"));
    m_access->backend()->execSql(QString("DROP INDEX dir_index;"));
    m_access->backend()->execSql(QString("DROP INDEX tag_index;"));

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Prepared table creation"));
    kDebug(50003) << "Dropped triggers" << endl;

    // --- Create new tables ---

    if (!createTablesV5() || !createIndicesV5())
        return false;

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Created tables"));

    // --- Populate AlbumRoots (from config) ---

    KSharedConfigPtr config = KGlobal::config();

    KConfigGroup group = config->group("Album Settings");
    QString albumLibraryPath = group.readEntry("Album Path", QString());

    if (albumLibraryPath.isEmpty())
    {
        kError(50003) << "Album library path from config file is empty. Aborting update." << endl;
        QString errorMsg = i18n("No album library path has been found in the configuration file. "
                                "Giving up the schema updating process. "
                                "Please try with an empty database, or repair your configuration.");
        m_access->setLastError(errorMsg);
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
        kError(50003) << "Failure to create a collection location. Aborting update." << endl;
        QString errorMsg = i18n("There was an error associating your albumLibraryPath (\"%1\") "
                                "with a storage volume of your system. "
                                "This problem may indicate that there is a problem with your installation. "
                                "If you are working on Linux, check that HAL is installed and running. "
                                "In any case, you can seek advice from the digikam-devel@kde.org mailing list. "
                                "The database updating process will now be aborted because we do not want "
                                "to create a new database based on false assumptions from a broken installation.");
        m_access->setLastError(errorMsg);
        m_setError = true;
        if (m_observer)
        {
            m_observer->error(errorMsg);
            m_observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }
        return false;
    }

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Configured one album root"));
    kDebug(50003) << "Inserted album root" << endl;

    // --- With the album root, populate albums ---

    if (!m_access->backend()->execSql(QString(
                    "REPLACE INTO Albums "
                    " (id, albumRoot, relativePath, date, caption, collection, icon) "
                    "SELECT id, ?, url, date, caption, collection, icon "
                    " FROM AlbumsV3;"
                                             ),
                    location.id())
       )
        return false;

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Imported albums"));
    kDebug(50003) << "Populated albums" << endl;

    // --- Add images ---

    if (!m_access->backend()->execSql(QString(
                    "REPLACE INTO Images "
                    " (id, album, name, status, category, modificationDate, fileSize, uniqueHash) "
                    "SELECT id, dirid, name, ?, ?, NULL, NULL, NULL"
                    " FROM ImagesV3;"
                                             ),
                    DatabaseItem::Visible, DatabaseItem::UndefinedCategory)
       )
         return false;

    // remove orphan images that would not be removed by CollectionScanner
    m_access->backend()->execSql(QString("DELETE FROM Images WHERE album NOT IN (SELECT id FROM Albums);"));

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Imported images information"));

    kDebug(50003) << "Populated Images" << endl;

    // --- Port searches ---

    if (!m_access->backend()->execSql(QString(
                    "REPLACE INTO Searches "
                    " (id, type, name, query) "
                    "SELECT id, ?, name, url"
                    " FROM SearchesV3;"),
                    DatabaseSearch::LegacyUrlSearch)
       )
         return false;

    SearchInfo::List sList = m_access->db()->scanSearches();

    for (SearchInfo::List::iterator it = sList.begin(); it != sList.end(); ++it)
    {
        KUrl url((*it).query);

        ImageQueryBuilder builder;
        QString query = builder.convertFromUrlToXml(url);

        QString name = (*it).name;
        if (name == i18n("Last Search"))
            name = i18n("Last Search (0.9)");

        if (url.queryItem("type") == QString("datesearch"))
        {
            m_access->db()->updateSearch((*it).id, DatabaseSearch::TimeLineSearch, name, query);
        }
        else if (url.queryItem("1.key") == "keyword")
        {
            m_access->db()->updateSearch((*it).id, DatabaseSearch::KeywordSearch, name, query);
        }
        else
        {
            m_access->db()->updateSearch((*it).id, DatabaseSearch::AdvancedSearch, name, query);
        }
    }

    // --- Create triggers ---

    if (!createTriggersV5())
        return false;
    kDebug(50003) << "Created triggers" << endl;

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

    m_access->db()->setUserFilterSettings(configImageFilter.toList(), configVideoFilter.toList(), configAudioFilter.toList());
    kDebug(50003) << "Set initial filter settings with user settings" << configImageFilter << endl;

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Initialized and imported file suffix filter"));

    // --- do a full scan ---

    CollectionScanner scanner;
    if (m_observer)
        m_observer->connectCollectionScanner(&scanner);
    scanner.completeScan();

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Did the initial full scan"));

    // --- Port date, comment and rating (_after_ the scan) ---

    // Port ImagesV3.date -> ImageInformation.creationDate
    if (!m_access->backend()->execSql(QString(
                    "UPDATE ImageInformation SET "
                    " creationDate=(SELECT datetime FROM ImagesV3 WHERE ImagesV3.id=ImageInformation.imageid) "
                    "WHERE imageid IN (SELECT id FROM ImagesV3);"
                                             )
                                     )
       )
         return false;

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Imported creation dates"));

    // Port ImagesV3.comment to ImageComments
    if (!m_access->backend()->execSql(QString(
                    "REPLACE INTO ImageComments "
                    " (imageid, type, language, comment) "
                    "SELECT id, ?, ?, caption FROM ImagesV3;"
                                             ),
                    (int)DatabaseComment::Comment, QString("x-default"))
       )
         return false;

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Imported comments"));

    // Port rating storage in ImageProperties to ImageInformation
    if (!m_access->backend()->execSql(QString(
                    "UPDATE ImageInformation SET "
                    " rating=(SELECT value FROM ImageProperties "
                    "         WHERE ImageInformation.imageid=ImageProperties.imageid AND ImageProperties.property=?) "
                    "WHERE imageid IN (SELECT imageid FROM ImageProperties WHERE property=?);"
                                             ),
                    QString("Rating"), QString("Rating"))
       )
         return false;

    m_access->backend()->execSql(QString("DELETE FROM ImageProperties WHERE property=?;"), QString("Rating"));
    m_access->backend()->execSql(QString("UPDATE ImageInformation SET rating=0 WHERE rating<0;"));

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Imported ratings"));

    // --- Drop old tables ---

    m_access->backend()->execSql(QString("DROP TABLE ImagesV3;"));
    m_access->backend()->execSql(QString("DROP TABLE AlbumsV3;"));
    m_access->backend()->execSql(QString("DROP TABLE SearchesV3;"));

    if (m_observer)
        m_observer->schemaUpdateProgress(i18n("Dropped v3 tables"));

    m_currentVersion = 5;
    kDebug(50003) << "Returning true from updating to 5" << endl;
    return true;
}

void SchemaUpdater::preAlpha010Update1()
{
    QString hasUpdate = m_access->db()->getSetting("preAlpha010Update1");
    if (!hasUpdate.isNull())
        return;

    if (!m_access->backend()->execSql(QString("ALTER TABLE Searches RENAME TO SearchesV3;")))
        return;

    if ( !m_access->backend()->execSql(
                   QString( "CREATE TABLE IF NOT EXISTS Searches  \n"
                            " (id INTEGER PRIMARY KEY, \n"
                            "  type INTEGER, \n"
                            "  name TEXT NOT NULL, \n"
                            "  query TEXT NOT NULL);" ) ))
        return;

    if (!m_access->backend()->execSql(QString(
                    "REPLACE INTO Searches "
                    " (id, type, name, query) "
                    "SELECT id, ?, name, url"
                    " FROM SearchesV3;"),
                    DatabaseSearch::LegacyUrlSearch)
       )
         return;

    SearchInfo::List sList = m_access->db()->scanSearches();

    for (SearchInfo::List::iterator it = sList.begin(); it != sList.end(); ++it)
    {
        KUrl url((*it).query);

        ImageQueryBuilder builder;
        QString query = builder.convertFromUrlToXml(url);

        if (url.queryItem("type") == QString("datesearch"))
        {
            m_access->db()->updateSearch((*it).id, DatabaseSearch::TimeLineSearch, (*it).name, query);
        }
        else if (url.queryItem("1.key") == "keyword")
        {
            m_access->db()->updateSearch((*it).id, DatabaseSearch::KeywordSearch, (*it).name, query);
        }
        else
        {
            m_access->db()->updateSearch((*it).id, DatabaseSearch::AdvancedSearch, (*it).name, query);
        }
    }

    m_access->backend()->execSql(QString("DROP TABLE SearchesV3;"));

    m_access->db()->setSetting("preAlpha010Update1", "true");
}

void SchemaUpdater::preAlpha010Update2()
{
    QString hasUpdate = m_access->db()->getSetting("preAlpha010Update2");
    if (!hasUpdate.isNull())
        return;

    if (!m_access->backend()->execSql(QString("ALTER TABLE ImagePositions RENAME TO ImagePositionsTemp;")))
        return;
    if (!m_access->backend()->execSql(QString("ALTER TABLE ImageMetadata RENAME TO ImageMetadataTemp;")))
        return;

    m_access->backend()->execSql(
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

    m_access->backend()->execSql(QString(
                    "REPLACE INTO ImagePositions "
                    " (imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                    "  altitude, orientation, tilt, roll, accuracy, description) "
                    "SELECT imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                    "  altitude, orientation, tilt, roll, 0, description "
                    " FROM ImagePositionsTemp;"));

    m_access->backend()->execSql(
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

    m_access->backend()->execSql( QString("INSERT INTO ImageMetadata "
                            " (imageid, make, model, lens, aperture, focalLength, focalLength35, "
                            "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                            "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) "
                            "SELECT imageid, make, model, NULL, aperture, focalLength, focalLength35, "
                            "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                            "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory "
                            "FROM ImageMetadataTemp;"));

    m_access->backend()->execSql(QString("DROP TABLE ImagePositionsTemp;"));
    m_access->backend()->execSql(QString("DROP TABLE ImageMetadataTemp;"));

    m_access->db()->setSetting("preAlpha010Update2", "true");
}

void SchemaUpdater::preAlpha010Update3()
{
    QString hasUpdate = m_access->db()->getSetting("preAlpha010Update3");
    if (!hasUpdate.isNull())
        return;

    m_access->backend()->execSql(QString("DROP TABLE ImageCopyright;"));
    m_access->backend()->execSql(
                    QString("CREATE TABLE ImageCopyright\n"
                            " (imageid INTEGER,\n"
                            "  property TEXT,\n"
                            "  value TEXT,\n"
                            "  extraValue TEXT,\n"
                            "  UNIQUE(imageid, property, value, extraValue));")
                                );

    m_access->db()->setSetting("preAlpha010Update3", "true");
}

void SchemaUpdater::beta010Update1()
{
    QString hasUpdate = m_access->db()->getSetting("beta010Update1");
    if (!hasUpdate.isNull())
        return;

    // if Image has been deleted
    m_access->backend()->execSql("DROP TRIGGER delete_image;");
    m_access->backend()->execSql(
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


    m_access->db()->setSetting("beta010Update1", "true");
}

void SchemaUpdater::beta010Update2()
{
    QString hasUpdate = m_access->db()->getSetting("beta010Update2");
    if (!hasUpdate.isNull())
        return;

    // force rescan and creation of ImageInformation entry for videos and audio
    m_access->backend()->execSql("DELETE FROM Images WHERE category=2 OR category=3;");

    m_access->db()->setSetting("beta010Update2", "true");
}

// ---------- Legacy code ------------


bool SchemaUpdater::createTablesV3()
{
    if (!m_access->backend()->execSql( QString("CREATE TABLE Albums\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  url TEXT NOT NULL UNIQUE,\n"
                            "  date DATE NOT NULL,\n"
                            "  caption TEXT,\n"
                            "  collection TEXT,\n"
                            "  icon INTEGER);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql( QString("CREATE TABLE Tags\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  pid INTEGER,\n"
                            "  name TEXT NOT NULL,\n"
                            "  icon INTEGER,\n"
                            "  iconkde TEXT,\n"
                            "  UNIQUE (name, pid));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql( QString("CREATE TABLE TagsTree\n"
                            " (id INTEGER NOT NULL,\n"
                            "  pid INTEGER NOT NULL,\n"
                            "  UNIQUE (id, pid));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql( QString("CREATE TABLE Images\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  name TEXT NOT NULL,\n"
                            "  dirid INTEGER NOT NULL,\n"
                            "  caption TEXT,\n"
                            "  datetime DATETIME,\n"
                            "  UNIQUE (name, dirid));") ))
    {
        return false;
    }


    if (!m_access->backend()->execSql( QString("CREATE TABLE ImageTags\n"
                            " (imageid INTEGER NOT NULL,\n"
                            "  tagid INTEGER NOT NULL,\n"
                            "  UNIQUE (imageid, tagid));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql( QString("CREATE TABLE ImageProperties\n"
                            " (imageid  INTEGER NOT NULL,\n"
                            "  property TEXT    NOT NULL,\n"
                            "  value    TEXT    NOT NULL,\n"
                            "  UNIQUE (imageid, property));") ))
    {
        return false;
    }

    if ( !m_access->backend()->execSql( QString( "CREATE TABLE Searches  \n"
                            " (id INTEGER PRIMARY KEY, \n"
                            "  name TEXT NOT NULL UNIQUE, \n"
                            "  url  TEXT NOT NULL);" ) ) )
    {
        return false;
    }

    if (!m_access->backend()->execSql( QString("CREATE TABLE Settings         \n"
                            "(keyword TEXT NOT NULL UNIQUE,\n"
                            " value TEXT);") ))
    {
        return false;
    }

    // TODO: see which more indices are needed
    // create indices
    m_access->backend()->execSql("CREATE INDEX dir_index ON Images    (dirid);");
    m_access->backend()->execSql("CREATE INDEX tag_index ON ImageTags (tagid);");

    // create triggers

    // trigger: delete from Images/ImageTags/ImageProperties
    // if Album has been deleted
    m_access->backend()->execSql("CREATE TRIGGER delete_album DELETE ON Albums\n"
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
    m_access->backend()->execSql("CREATE TRIGGER delete_image DELETE ON Images\n"
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
    m_access->backend()->execSql("CREATE TRIGGER delete_tag DELETE ON Tags\n"
            "BEGIN\n"
            "  DELETE FROM ImageTags WHERE tagid=OLD.id;\n"
            "END;");

    // trigger: insert into TagsTree if Tag has been added
    m_access->backend()->execSql("CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags\n"
            "BEGIN\n"
            "  INSERT INTO TagsTree\n"
            "    SELECT NEW.id, NEW.pid\n"
            "    UNION\n"
            "    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;\n"
            "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    m_access->backend()->execSql("CREATE TRIGGER delete_tagstree DELETE ON Tags\n"
            "BEGIN\n"
            " DELETE FROM Tags\n"
            "   WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
            " DELETE FROM TagsTree\n"
            "   WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
            " DELETE FROM TagsTree\n"
            "    WHERE id=OLD.id;\n"
            "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    m_access->backend()->execSql("CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags\n"
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
