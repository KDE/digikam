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

// Local includes

#include "ddebug.h"
#include "databasebackend.h"
#include "albumdb.h"
#include "databasetransaction.h"
#include "upgradedb_sqlite2tosqlite3.h"
#include "schemaupdater.h"

namespace Digikam
{

int SchemaUpdater::schemaVersion()
{
    //return 4;
    return 1;
}

SchemaUpdater::SchemaUpdater(DatabaseAccess *access)
{
    m_access = access;
    m_currentVersion = 0;
}

bool SchemaUpdater::update()
{
    DatabaseTransaction transaction(m_access);
    bool success = makeUpdates();
    m_access->db()->setSetting("DBVersion",QString::number(m_currentVersion));
    return success;
}

bool SchemaUpdater::makeUpdates()
{
    // First step: do we have an empty database?
    QStringList tables = m_access->backend()->tables();

    if (tables.contains("Albums"))
    {
        // Find out schema version of db file
        QString version = m_access->db()->getSetting("DBVersion");
        if (version.isNull())
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
        if (m_currentVersion < schemaVersion())
        {
            // insert here future schema upgrades
        }
        return true;
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
        // Version 4 is 0.10, the digikam3.db file copied to digikam4.db,
        //  no schema changes.
        // Version 3 wrote the setting "DBVersion", "1",
        // Version 4 writes "4".

        /*
        DatabaseParameters parameters = m_access->parameters();
        if (parameters.isSQLite())
        {
            QFileInfo currentDBFile(parameters.databaseName);
            QFileInfo digikam3DB(currentDBFile.dir(), "digikam3.db");
            QFileInfo digikamDB(currentDBFile.dir(), "digikam.db");

            if (digikam3DB.exists())
            {
                return copyV3toV4(digikam3DB.filePath(), currentDBFile.filePath());
            }
            else if (digikamDB.exists())
            {
                return updateV2toV4(digikamDB.path());
            }
        }
        */

        // No legacy handling: start with a fresh db
        if (!createTables())
        {
            m_access->setLastError(i18n("Failed to create tables on database.\n ")
                                   + m_access->backend()->lastError());
            return false;
        }
        return true;
    }
}

bool SchemaUpdater::createTables()
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
        return false;
    else
    {
        m_currentVersion = 1;
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

bool SchemaUpdater::createTablesV5()
{
    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE AlbumRoots\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  status INTEGER NOT NULL,\n"
                            "  absolutePath TEXT,\n"
                            "  type INTEGER NOT NULL,\n"
                            "  volumeUuid TEXT,\n"
                            "  specificPath TEXT);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE Albums\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  albumRoot INTEGER NOT NULL FOREIGN KEY,\n"
                            "  relativePath TEXT NOT NULL UNIQUE,\n"
                            "  date DATE,\n"
                            "  caption TEXT,\n"
                            "  collection TEXT,\n"
                            "  icon INTEGER);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE Tags\n"
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
                    QString("CREATE TABLE TagsTree\n"
                            " (id INTEGER NOT NULL,\n"
                            "  pid INTEGER NOT NULL,\n"
                            "  UNIQUE (id, pid));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE Images\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  album INTEGER,\n" // no constraints, for temporary orphans
                            "  name TEXT NOT NULL,\n"
                            "  status INTEGER,\n"
                            "  modificationDate DATETIME,\n"
                            "  fileSize INTEGER,\n"
                            "  uniqueHash TEXT,\n"
                            "  UNIQUE (albumRoot, album, name));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageHaarMatrix\n"
                            " (imageid INTEGER UNIQUE FOREIGN KEY,\n"
                            "  matrix BLOB);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageInformation\n"
                            " (imageid INTEGER UNIQUE FOREIGN KEY,\n"
                            "  rating INTEGER,\n"
                            "  creationDate DATETIME,\n"
                            "  digitizationDate DATETIME,\n"
                            "  orientation INTEGER,\n"
                            "  sizeX INTEGER,\n"
                            "  sizeY INTEGER,\n"
                            "  colorDepth INTEGER,\n"
                            "  colorModel INTEGER);") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageMetadata\n"
                            " (imageid INTEGER UNIQUE FOREIGN KEY,\n"
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
                            "  subjectDistanceCategory INTEGER);"
                           ) ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImagePositions\n"
                            " (imageid INTEGER UNIQUE FOREIGN KEY,\n"
                            "  latitude TEXT,\n"
                            "  latitudeNumber REAL,\n"
                            "  longitude TEXT,\n"
                            "  longitudeNumber REAL,\n"
                            "  altitude REAL,\n"
                            "  orientation REAL,\n"
                            "  tilt REAL,\n"
                            "  roll REAL,\n"
                            "  description TEXT);"
                           ) ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageComments\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  imageid INTEGER UNIQUE FOREIGN KEY,\n"
                            "  source INTEGER,\n"
                            "  language TEXT,\n"
                            "  author TEXT,\n"
                            "  date DATETIME,\n"
                            "  comment TEXT,\n"
                            "  UNIQUE(imageid, source, language, author));"
                           ) ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageCopyright\n"
                            " (imageid INTEGER UNIQUE FOREIGN KEY,\n"
                            "  property TEXT,\n"
                            "  value TEXT,\n"
                            "  extraValue TEXT);"
                           ) ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageTags\n"
                            " (imageid INTEGER NOT NULL,\n"
                            "  tagid INTEGER NOT NULL,\n"
                            "  UNIQUE (imageid, tagid));") ))
    {
        return false;
    }

    if (!m_access->backend()->execSql(
                    QString("CREATE TABLE ImageProperties\n"
                            " (imageid  INTEGER NOT NULL,\n"
                            "  property TEXT    NOT NULL,\n"
                            "  value    TEXT    NOT NULL,\n"
                            "  UNIQUE (imageid, property));") ))
    {
        return false;
    }

    if ( !m_access->backend()->execSql(
                   QString( "CREATE TABLE Searches  \n"
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
                    QString("CREATE TABLE Settings         \n"
                            "(keyword TEXT NOT NULL UNIQUE,\n"
                            " value TEXT);") ))
        return false;
    else
    {
        m_currentVersion = 1;
    }

    // TODO: see which more indices are needed
    // create indices
    m_access->backend()->execSql("CREATE INDEX dir_index ON Images    (dirid);");
    m_access->backend()->execSql("CREATE INDEX tag_index ON ImageTags (tagid);");

    // Create triggers

    // Triggers for deletion of images

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

bool SchemaUpdater::createTablesV3()
{
    return createTables();
}

}  // namespace Digikam
