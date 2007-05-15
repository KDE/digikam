/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 * 
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


// Local includes

#include "databasebackend.h"
#include "schemaupdater.h"

namespace Digikam
{


int SchemaUpdater::schemaVersion()
{
    return 1;
}

SchemaUpdater::SchemaUpdater(DatabaseBackend *backend)
    : m_backend(backend)
{
}

bool SchemaUpdater::update()
{
    //TODO: move here SQLite2ToSQLite3?
    QStringList values;

    if (!m_backend->execSql( QString("SELECT name FROM sqlite_master"
                                     " WHERE type='table'"
                                     " ORDER BY name;"),
                             &values ))
    {
        return false;
    }

    if (!values.contains("Albums"))
        return createTables();
    else
        return true;
}

bool SchemaUpdater::createTables()
{
    if (!m_backend->execSql( QString("CREATE TABLE Albums\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  url TEXT NOT NULL UNIQUE,\n"
                            "  date DATE NOT NULL,\n"
                            "  caption TEXT,\n"
                            "  collection TEXT,\n"
                            "  icon INTEGER);") ))
    {
        return false;
    }

    if (!m_backend->execSql( QString("CREATE TABLE Tags\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  pid INTEGER,\n"
                            "  name TEXT NOT NULL,\n"
                            "  icon INTEGER,\n"
                            "  iconkde TEXT,\n"
                            "  UNIQUE (name, pid));") ))
    {
        return false;
    }

    if (!m_backend->execSql( QString("CREATE TABLE TagsTree\n"
                            " (id INTEGER NOT NULL,\n"
                            "  pid INTEGER NOT NULL,\n"
                            "  UNIQUE (id, pid));") ))
    {
        return false;
    }

    if (!m_backend->execSql( QString("CREATE TABLE Images\n"
                            " (id INTEGER PRIMARY KEY,\n"
                            "  name TEXT NOT NULL,\n"
                            "  dirid INTEGER NOT NULL,\n"
                            "  caption TEXT,\n"
                            "  datetime DATETIME,\n"
                            "  UNIQUE (name, dirid));") ))
    {
        return false;
    }


    if (!m_backend->execSql( QString("CREATE TABLE ImageTags\n"
                            " (imageid INTEGER NOT NULL,\n"
                            "  tagid INTEGER NOT NULL,\n"
                            "  UNIQUE (imageid, tagid));") ))
    {
        return false;
    }

    if (!m_backend->execSql( QString("CREATE TABLE ImageProperties\n"
                            " (imageid  INTEGER NOT NULL,\n"
                            "  property TEXT    NOT NULL,\n"
                            "  value    TEXT    NOT NULL,\n"
                            "  UNIQUE (imageid, property));") ))
    {
        return false;
    }

    if ( !m_backend->execSql( QString( "CREATE TABLE Searches  \n"
                            " (id INTEGER PRIMARY KEY, \n"
                            "  name TEXT NOT NULL UNIQUE, \n"
                            "  url  TEXT NOT NULL);" ) ) )
    {
        return false;
    }

    if (!m_backend->execSql( QString("CREATE TABLE Settings         \n"
                            "(keyword TEXT NOT NULL UNIQUE,\n"
                            " value TEXT);") ))
        return false;
    else
    {
        //m_backend->setSetting("DBVersion","1");
        m_backend->execSql( QString("REPLACE into Settings VALUES ('DBVersion','1');") );
    }

    // TODO: see which more indices are needed
    // create indices
    m_backend->execSql("CREATE INDEX dir_index ON Images    (dirid);");
    m_backend->execSql("CREATE INDEX tag_index ON ImageTags (tagid);");

    // create triggers

    // trigger: delete from Images/ImageTags/ImageProperties
    // if Album has been deleted
    m_backend->execSql("CREATE TRIGGER delete_album DELETE ON Albums\n"
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
    m_backend->execSql("CREATE TRIGGER delete_image DELETE ON Images\n"
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
    m_backend->execSql("CREATE TRIGGER delete_tag DELETE ON Tags\n"
            "BEGIN\n"
            "  DELETE FROM ImageTags WHERE tagid=OLD.id;\n"
            "END;");

    // trigger: insert into TagsTree if Tag has been added
    m_backend->execSql("CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags\n"
            "BEGIN\n"
            "  INSERT INTO TagsTree\n"
            "    SELECT NEW.id, NEW.pid\n"
            "    UNION\n"
            "    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;\n"
            "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    m_backend->execSql("CREATE TRIGGER delete_tagstree DELETE ON Tags\n"
            "BEGIN\n"
            " DELETE FROM Tags\n"
            "   WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
            " DELETE FROM TagsTree\n"
            "   WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
            " DELETE FROM TagsTree\n"
            "    WHERE id=OLD.id;\n"
            "END;");

    // trigger: delete from TagsTree if Tag has been deleted
    m_backend->execSql("CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags\n"
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

}

