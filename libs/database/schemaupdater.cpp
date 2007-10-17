/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 * 
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>

// Local includes

#include "ddebug.h"
#include "databasebackend.h"
#include "albumdb.h"
#include "databasetransaction.h"
#include "upgradedb_sqlite2tosqlite3.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionscanner.h"
#include "schemaupdater.h"

namespace Digikam
{

int SchemaUpdater::schemaVersion()
{
    return 5;
}

int SchemaUpdater::filterSettingsVersion()
{
    return 1;
}

SchemaUpdater::SchemaUpdater(DatabaseAccess *access)
{
    m_access = access;
    m_currentVersion = 0;
}

bool SchemaUpdater::update()
{
    bool success = startUpdates();
    m_access->db()->setSetting("DBVersion",QString::number(m_currentVersion));
    updateFilterSettings();
    return success;
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

        // We absolutely require the DBVersion setting
        if (version.isEmpty())
        {
            // Something is damaged. Give up.
            DError() << "DBVersion not available! Giving up schema upgrading." << endl;
            m_access->setLastError(i18n(
                    "The database is not valid: "
                    "The \"DBVersion\" setting does not exist. "
                    "The current database schema version cannot be verified. "
                    "Try to start with an empty database. "
                                   ));
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
                m_access->setLastError(i18n(
                            "The database has been used with a more recent version of digiKam "
                            "and has been updated to a database schema which cannot be used with this version. "
                            "(This means this digiKam version is too old, or the database format is to recent) "
                            "Please use the more recent version of digikam that you used before. "
                                           ));
                return false;
            }
        }
        else
            return makeUpdates();
    }
    else
    {
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
            }
            else if (digikamDB.exists())
            {
                if (!updateV2toV4(digikamDB.path()))
                    return false;
            }

            // m_currentVersion is now 4;
            return makeUpdates();
        }

        // No legacy handling: start with a fresh db
        if (!createDatabase())
        {
            m_access->setLastError(i18n("Failed to create tables on database.\n ")
                                   + m_access->backend()->lastError());
            return false;
        }
        return true;
    }
}

bool SchemaUpdater::makeUpdates()
{
    //DatabaseTransaction transaction(m_access);
    if (m_currentVersion < schemaVersion())
    {
        if (m_currentVersion < 5)
        {
            m_access->backend()->beginTransaction();
            if (!updateV4toV5())
            {
                m_access->backend()->rollbackTransaction();
                return false;
            }
            m_access->backend()->commitTransaction();
        }
        // add future updates here
    }
    return true;
}

bool SchemaUpdater::updateFilterSettings()
{
    // No updates available so far, initial settings done in V4toV5
    /*
    QString filterVersion = m_access->db()->getSetting("FilterSettingsVersion");
    if (!filterSettingsVersion.isEmpty())
    {
        int version = filterVersion.toInt();
        if (version < filterSettingsVersion())
        {
        }
    }
    */
    return true;
}

bool SchemaUpdater::createDatabase()
{
    return createTablesV5()
        && createIndicesV5()
        && createTriggersV5();
}

bool SchemaUpdater::createTablesV5()
{
    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE AlbumRoots\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  status INTEGER NOT NULL,\n"
                            "  type INTEGER NOT NULL,\n"
                            "  identifier TEXT,\n"
                            "  specificPath TEXT);") ));
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE Albums\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  albumRoot INTEGER NOT NULL,\n"
                            "  relativePath TEXT NOT NULL UNIQUE,\n"
                            "  date DATE,\n"
                            "  caption TEXT,\n"
                            "  collection TEXT,\n"
                            "  icon INTEGER);") ))
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
                            " (imageid INTEGER UNIQUE PRIMARY KEY,\n"
                            "  matrix BLOB);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageInformation\n"
                            " (imageid INTEGER UNIQUE PRIMARY KEY,\n"
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
                            " (imageid INTEGER UNIQUE PRIMARY KEY,\n"
                            "  make TEXT,\n"
                            "  model TEXT,\n"
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
                            " (imageid INTEGER UNIQUE PRIMARY KEY,\n"
                            "  latitude TEXT,\n"
                            "  latitudeNumber REAL,\n"
                            "  longitude TEXT,\n"
                            "  longitudeNumber REAL,\n"
                            "  altitude REAL,\n"
                            "  orientation REAL,\n"
                            "  tilt REAL,\n"
                            "  roll REAL,\n"
                            "  description TEXT);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageComments\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  imageid INTEGER UNIQUE,\n"
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
                            " (imageid INTEGER,\n"
                            "  property TEXT,\n"
                            "  value TEXT,\n"
                            "  extraValue TEXT);") ))
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
                            "  name TEXT NOT NULL UNIQUE, \n"
                            "  url  TEXT NOT NULL);" ) ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE DownloadHistory\n"
                            " (id  INTEGER PRIMARY KEY,\n"
                            "  filepath TEXT,\n"
                            "  filename TEXT,\n"
                            "  filesize INTEGER,\n"
                            "  filedate DATETIME);"
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
    // Triggers for deletion of images

    // trigger: delete from Images/ImageTags/ImageProperties
    // if Album has been deleted
    m_access->backend()->execSql("CREATE TRIGGER delete_album DELETE ON Albums\n"
            "BEGIN\n"
            " DELETE FROM ImageTags\n"
            "   WHERE imageid IN (SELECT id FROM Images WHERE album=OLD.id);\n"
            " DELETE From ImageProperties\n"
            "   WHERE imageid IN (SELECT id FROM Images WHERE album=OLD.id);\n"
            " DELETE FROM Images\n"
            "   WHERE dirid = OLD.id;\n"
            "END;");

    // trigger: delete from ImageTags/ImageProperties
    // if Image has been deleted
    m_access->backend()->execSql(
            "CREATE TRIGGER delete_image DELETE ON Images\n"
            "BEGIN\n"
            "  DELETE FROM ImageTags\n"
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
    m_access->backend()->close();

    KUrl digikam3DBUrl, currentDBUrl;
    digikam3DBUrl.setPath(digikam3DBPath);
    currentDBUrl.setPath(currentDBPath);

    KIO::Job *job = KIO::file_copy(digikam3DBUrl, currentDBUrl, -1, KIO::Overwrite);
    if (!KIO::NetAccess::synchronousRun(job, 0))
    {
        m_access->setLastError(i18n("Failed to copy the old database file (\"%1\")"
                                    "to its new location (\"%2\")."
                                    "Please make sure that the file can be copied.", 
                                    digikam3DBPath, currentDBPath));
    }

    if (!m_access->backend()->open(m_access->parameters()))
    {
        m_access->setLastError(i18n("The old database file (\"%1\") has been copied "
                                    "to the new location (\"%2\") but it cannot be opened. "
                                    "Please remove both files and try again, "
                                    "starting with an empty database. ",
                                    digikam3DBPath, currentDBPath));

        return false;
    }
    m_currentVersion = 4;
    return true;
}

bool SchemaUpdater::updateV2toV4(const QString &sqlite2DBPath)
{
    if (upgradeDB_Sqlite2ToSqlite3(*m_access, sqlite2DBPath))
    {
        m_currentVersion = 4;
        return true;
    }
    else
    {
        m_access->setLastError(i18n("Could not update from the old SQLite2 file (\"%1\"). "
                                    "Please remove this file and try again, "
                                    "starting with an empty database. ", sqlite2DBPath));
        return false;
    }
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
    foreach (QString f, sepList)
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
    // This update was introduced from digikam version 0.9 to digikam 0.10
    // We operator on an SQLite3 database under a transaction (which will be rolled back on error)

    // --- Make space for new tables ---
    if (!m_access->backend()->execSql(QString("ALTER TABLE Albums RENAME TO AlbumsV3;")))
        return false;

    if (!m_access->backend()->execSql(QString("ALTER TABLE Images RENAME TO ImagesV3;")))
        return false;

    // --- Drop some triggers and indices ---

    // Dont check for errors here. The "IF EXISTS" clauses seem not supported in SQLite
    m_access->backend()->execSql(QString("DROP TRIGGER delete_album;"));
    m_access->backend()->execSql(QString("DROP TRIGGER delete_image;"));
    m_access->backend()->execSql(QString("DROP TRIGGER delete_tag;"));
    m_access->backend()->execSql(QString("DROP TRIGGER insert_tagstree;"));
    m_access->backend()->execSql(QString("DROP TRIGGER delete_tagstree;"));
    m_access->backend()->execSql(QString("DROP TRIGGER move_tagstree;"));
    m_access->backend()->execSql(QString("DROP INDEX dir_index;"));

    // --- Create new tables ---

    if (!createTablesV5() && createIndicesV5())
        return false;

    // --- Populate AlbumRoots (from config) ---

    KSharedConfigPtr config = KGlobal::config();

    KConfigGroup group = config->group("Album Settings");
    QString albumLibraryPath = group.readEntry("Album Path", QString());

    if (albumLibraryPath.isEmpty())
    {
        m_access->setLastError(i18n("No album library path has been found in the configuration file. "
                                    "Giving up the schema updating process. "
                                    "Please try with an empty database, or repair your configuration."));
        return false;
    }

    CollectionLocation *location =
            CollectionManager::instance()->addLocation(KUrl::fromPath(albumLibraryPath));
    if (!location)
        return false;

    // --- With the album root, populate albums ---

    if (!m_access->backend()->execSql(QString(
                    "REPLACE INTO Albums "
                    " (id, albumRoot, relativePath, date, caption, collection, icon) "
                    "SELECT id, ?, url, date, caption, collection, icon "
                    " FROM AlbumsV3;"
                                             ),
                    location->id())
       )
        return false;

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

    // --- Create triggers ---

    if (!createTriggersV5())
        return false;

    // --- Populate name filters ---

    QStringList defaultImageFilter, defaultVideoFilter, defaultAudioFilter;

    defaultImageFilter << "jpg" << "jpeg" << "jpe"               // JPEG
                       << "jp2" << "jpx"  << "jpc" << "pgx"      // JPEG-2000
                       << "tif" << "tiff"                        // TIFF
                       << "png"                                  // PNG
                       << "xpm" << "ppm" << "pnm"
                       << "gif" << "bmp" << "xcf" << "pcx";

    // RAW file extentions supported by dcraw 8.77
    // This information belongs to libkdcraw, but here at least this will be included statically.
    defaultImageFilter << "bay" << "bmq" << "cr2" << "crw" << "cs1"
                       << "dc2" << "dcr" << "dng" << "erf" << "fff"
                       << "hdr" << "k25" << "kdc" << "mdc" << "mos"
                       << "mrw" << "nef" << "orf" << "pef" << "pxn"
                       << "raf" << "raw" << "rdc" << "sr2" << "srf"
                       << "x3f" << "arw";

    defaultVideoFilter << "mpeg" << "mpg" << "mpo" << "mpe"     // MPEG
                       << "avi"  << "mov" << "wmf" << "asf" << "mp4";

    defaultAudioFilter << "ogg" << "mp3" << "wma" << "wav";

    m_access->db()->setFilterSettings(defaultImageFilter, defaultVideoFilter, defaultAudioFilter);
    m_access->db()->setSetting("FilterSettingsVersion", "1");

    // Set user settings from config

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

    // --- do a full scan ---

    // TODO: Add UI!!!
    CollectionScanner scanner;
    scanner.completeScan();

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

    // Port ImagesV3.comment to ImageComments
    if (!m_access->backend()->execSql(QString(
                    "REPLACE INTO ImageComments "
                    " (imageid, type, language, comment) "
                    "SELECT id, ?, ?, caption FROM ImagesV3;"
                                             ),
                    (int)DatabaseComment::Comment, QString("x-default"))
       )
         return false;

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

    // --- Drop old tables ---

    m_access->backend()->execSql(QString("DROP TABLE ImagesV3;"));
    m_access->backend()->execSql(QString("DROP TABLE AlbumsV3;"));

    return true;
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
