/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-18
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <kdebug.h>
#include <klocale.h>

#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qtextstream.h>

#include <cstdio>
#include <cstdlib>

extern "C" {
#include <sqlite.h>
#include <sys/time.h>
#include <time.h>
}

#include "album.h"
#include "albummanager.h"
#include "albumdb.h"

typedef struct sqlite_vm sqlite_vm;

AlbumDB::AlbumDB()
{
    m_valid = false;
    m_db    = 0;
}

AlbumDB::~AlbumDB()
{
    removeInvalidEntries();
    
    if (m_db) {
        sqlite_close(m_db);
    }
}

void AlbumDB::setDBPath(const QString& path)
{
    if (m_db) {
        sqlite_close(m_db);
	m_db = 0;
    }

    m_valid = false;
    
    char *errMsg = 0;
    m_db = sqlite_open(QFile::encodeName(path), 0, &errMsg);
    if (m_db == 0)
    {
        kdWarning() << k_funcinfo << "Cannot open database: "
                    << errMsg << endl;
        free(errMsg);
    }
    else
    {
        initDB();
    }
}

void AlbumDB::initDB()
{
    m_valid = false;
    
    // Check if we have the required tables

    QStringList values;
    
    if (!execSql( QString("SELECT name FROM sqlite_master"
                          " WHERE type='table'"
                          " ORDER BY name;"),
                  &values ))
    {
        return;
    }

    if (!values.contains("Albums"))
    { 
        if (!execSql( QString("CREATE TABLE Albums"
                              " (id INTEGER PRIMARY KEY,"
			      "  url TEXT NOT NULL UNIQUE,"
                              "  date DATE NOT NULL,"
                              "  caption TEXT,"
                              "  collection TEXT,"
                              "  ignoreprops BOOLEAN,"
                              "  icon TEXT);") ))
        {
            return;
        }
	
        if (!execSql( QString("CREATE TABLE Tags"
                              " (id INTEGER PRIMARY KEY,"
			      "  pid INTEGER,"
			      "  name TEXT NOT NULL,"
                              "  icon TEXT,"
			      "  UNIQUE (name, pid));") ))
        {
            return;
        }

        if (!execSql( QString("CREATE TABLE Images"
                              " (name TEXT NOT NULL,"
                              "  dirid INTEGER NOT NULL,"
                              "  caption TEXT,"
                              "  datetime DATETIME,"
                              "  UNIQUE (name, dirid));") ))
        {
            return;
        }

 
        if (!execSql( QString("CREATE TABLE ImageTags"
                              " (name TEXT NOT NULL,"
                              "  dirid INTEGER NOT NULL,"
                              "  tagid INTEGER NOT NULL, "
                              " UNIQUE (name, dirid, tagid));") ))
        {
            return;
        }
        
        // create indices
        execSql("CREATE INDEX dir_index ON Images    (dirid);");
        execSql("CREATE INDEX tag_index ON ImageTags (tagid);");


        // create triggers

        // trigger: delete from ImageProps/ImageTags if Album has been deleted
        execSql("CREATE TRIGGER delete_album DELETE ON Albums\n"
                "BEGIN\n"
                " DELETE FROM Images     WHERE dirid = old.id;\n"
                " DELETE FROM ImageTags  WHERE dirid = old.id;\n"
                "END;");

        // trigger: delete from ImageTags if Tag has been deleted 
	// also delete tags whose parent are this
	// note: deleting the tag will recursively delete all subtags
        execSql("CREATE TRIGGER delete_tag DELETE ON Tags\n"
                "BEGIN\n"
                " DELETE FROM ImageTags WHERE tagid = old.id;\n"
                " DELETE FROM Tags       WHERE pid   = old.id;\n"
                "END;");


        // insert some initial tags
        execSql( QString("INSERT INTO Tags "
                         "VALUES(null, 0, '%1', 'tag-events');")
                 .arg(i18n("Events")) );
        execSql( QString("INSERT INTO Tags "
                         "VALUES(null, 0, '%1', 'tag-people');")
                 .arg(i18n("People")) );
        execSql( QString("INSERT INTO Tags "
                         "VALUES(null, 0, '%2', 'tag-places');")
                 .arg(i18n("Places")) );
    }
    
    m_valid = true;
}

void AlbumDB::readAlbum(Album* album)
{
    if (!album)
        return;

    switch(album->type()) {
    case(Album::PHYSICAL): {
        readPAlbum(dynamic_cast<PAlbum*>(album));
        break;
    }
    case(Album::TAG): {
        readTAlbum(dynamic_cast<TAlbum*>(album));
        break;
    }
    default:
	kdWarning() << k_funcinfo << "Unknown album type"
		    << endl;
    }
}

void AlbumDB::deleteAlbum(Album *a)
{
    if (!a)
        return;

    // make sure to delete all albums which are subalbums of this

    switch(a->type())
    {
    case(Album::PHYSICAL):
    {
        PAlbum *album = dynamic_cast<PAlbum*>(a);
        // deleting album
        execSql( QString("DELETE FROM Albums WHERE id='%1'")
                 .arg(album->getID()) );
        // deleting subalbums
        execSql( QString("DELETE FROM Albums WHERE url LIKE '%1/%'")
                 .arg(escapeString(album->getURL())) );
        break;
    }
    case(Album::TAG):
    {
        TAlbum *album = dynamic_cast<TAlbum*>(a);
        // deleting tag
        execSql( QString("DELETE FROM Tags WHERE id='%1'")
                 .arg(album->getID()) );
	// no need to delete subtags. trigger takes care of it
        break;
    }
    default:
        break;
    }
}

void AlbumDB::readPAlbum(PAlbum *album)
{
    if (!m_db)
        return;
    
    QStringList values;

    execSql( QString("SELECT id, date, caption, collection, icon "
                     "FROM Albums WHERE url='%1';")
             .arg(escapeString(album->getURL())),
             &values );

    if (!values.isEmpty()) 
    {
        album->setID(values[0].toInt());    
        album->setDate(QDate::fromString(values[1], Qt::ISODate), false);
        album->setCaption(values[2], false);
        album->setCollection(values[3], false);
        album->setIcon(values[4]);

        return;
    }

    kdDebug() << "Album not in database : " << album->getURL() << endl;

    // Turn of automatic renaming as it can create havoc if user changes locale
    /* 
    //  Album not in database. first check if it is an album which was in
    //  database, but has been copied/moved/renamed 

    int id;
    if (readIdentifier(album, id) && checkAlbum(album, id))
    {
        kdDebug() << k_funcinfo << "Successfully renamed album"
                  << album->getURL() << endl;
        return;
    }
    */
    
    // Smells like an entirely new album. go ahead and 
    // and add to database 

    
    execSql( QString("INSERT INTO Albums (url, date, caption, collection) "
                     "VALUES('%1', '%2', '%3', '%4');")
             .arg(escapeString(album->getURL()))
             .arg(album->getDate().toString(Qt::ISODate))
             .arg(escapeString(album->getCaption()))
             .arg(escapeString(album->getCollection())) );

    // Write the identifier of the album into the directory 

    int id = sqlite_last_insert_rowid(m_db);
    album->setID(id);

    // Turn of automatic renaming as it can create havoc if user changes locale
    /* writeIdentifier(album, id); */

    
    // try importing any old digikam.xml file 
    importXML(album);
    
}

void AlbumDB::readTAlbum(TAlbum* album)
{
    QStringList values;

    execSql( QString("SELECT id, icon FROM Tags WHERE url='%1';")
             .arg(escapeString(album->getURL())),
             &values );

    if (!values.isEmpty())
    {
	album->setID(values[0].toInt());    
        album->setIcon(values[1]);
    }
    else
    {
        /* new tag. add to database */
        execSql( QString("INSERT INTO Tags "
                         "VALUES(null, '%1', '%2');")
                 .arg(escapeString(album->getURL()))
                 .arg(escapeString(album->getIcon())) );
        album->setID(sqlite_last_insert_rowid(m_db));
    }
}

void AlbumDB::beginTransaction()
{
    execSql( "BEGIN TRANSACTION;" );
}

void AlbumDB::commitTransaction()
{
    execSql( "COMMIT TRANSACTION;" );
}

void AlbumDB::setCaption(PAlbum *album)
{
    execSql( QString("UPDATE Albums SET caption='%1' WHERE id=%2;")
             .arg(escapeString(album->getCaption()))
             .arg(album->getID()) );
}

void AlbumDB::setCollection(PAlbum *album)
{
    execSql( QString("UPDATE Albums SET collection='%1' WHERE id=%2;")
             .arg(escapeString(album->getCollection()))
             .arg(album->getID()) );
}

void AlbumDB::setDate(PAlbum *album)
{
    execSql( QString("UPDATE Albums SET date='%1' WHERE id = %2;")
             .arg(album->getDate().toString(Qt::ISODate))
             .arg(album->getID()) );
}

bool AlbumDB::importXML(PAlbum *album)
{
    if (!album)
        return false;

    QFile file(album->getKURL().path(1) + "digikam.xml");
    if (!file.exists() || !file.open(IO_ReadOnly))
    {
        return false;
    }

    QDomDocument doc("XMLAlbumProperties");
    if (!doc.setContent(&file))
    {
        kdWarning() << "AlbumDB:importXML: Failed to set content from xml file: "
                    << album->getTitle() << endl;
        file.close();
        return false;
    }
    file.close();
    
    QDomElement elem = doc.documentElement();
    if (elem.tagName() != "album")
    {
        return false;
    }

    QString caption    = elem.attribute("comments");
    QString collection = elem.attribute("collection");
    QString date       = elem.attribute("date");

    if (!caption.isNull())
        album->setCaption(caption, true);

    if (!collection.isNull())
        album->setCollection(collection, true);

    if (!date.isNull())
        album->setDate(QDate::fromString(date, Qt::ISODate), true);

    QDomNode node = elem.firstChild();
    if (!node.isNull() && !node.toElement().isNull())
    {
        elem = node.toElement();

        for (QDomNode n = elem.firstChild();
             !n.isNull(); n = n.nextSibling()) {

            QDomElement e = n.toElement();
            if (e.isNull()) continue;
            if (e.tagName() != "item") continue;

            QString name    = e.attribute("name");
            QString caption = e.attribute("comments");
            if (!name.isNull() && !caption.isNull()) {
                execSql( QString("INSERT INTO Images (name, dirid, caption) "
                                 "VALUES('%1','%2','%3');")
                         .arg(escapeString(name))
                         .arg(album->getID())
                         .arg(escapeString(caption)) );
            }
        }
    } 

    return true;
}

bool AlbumDB::exportXML(PAlbum *album)
{
    if (!album)
        return false;

    //TODO
    
    return false;
}

bool AlbumDB::readIdentifier(PAlbum *album, int& id)
{
    if (!album)
        return false;

    QFileInfo fi(album->getKURL().path(1) + ".directory");
    if (!fi.exists())
	return false;    

    KConfig config(album->getKURL().path(1) + ".directory");
    config.setGroup("Digikam");
    if (!config.hasKey("ID"))
	return false;

    id = config.readNumEntry("ID");
    
    return true;    
}

void AlbumDB::writeIdentifier(PAlbum *album, int id)
{
    if (!album)
        return;

    KConfig config(album->getKURL().path(1) + ".directory");
    config.setGroup("Digikam");
    config.writeEntry("ID", id);
}

bool AlbumDB::checkAlbum(PAlbum *album, int id)
{
    // Three possibilites:
    // a. album has been renamed/moved. 
    // b. album is a copy of an existing album (user copied the whole folder).
    // c. id is there for the album but not in database
    // Steps:
    // a. check first if the id exists in the db.
    // b. if not, its an entirely new album. return.
    // a. check if another album folder with same id already exists.
    // b. if yes, its a copy: insert into database a copy of the old
    //    album with the new album as the url and write a new id for it
    // c. if no, its a moved/renamed album: just update the url of the
    //    album for the id. 

    QStringList values;

    execSql( QString("SELECT url FROM Albums WHERE id = %1;")
                  .arg(id), &values );

    if ( !values.isEmpty() ) 
    {
        // id exists in the album.
        // check now if the folder for the url exists

        QString oldURL( values[0] );
        
        QDir dir( AlbumManager::instance()->getLibraryPath() +
                  oldURL );

        if ( dir.exists() ) {

            // folder exists. that means this album is a copy of the old ablum
            // copy the properties for the old album into the new album

            execSql( QString("INSERT INTO Albums (url, date, caption, collection, icon) "
                             "SELECT '%1', date, caption, collection, icon FROM Albums "
                             "WHERE id = %2;")
                     .arg(escapeString(album->getURL()))
                     .arg(id) );
            
	    // find identifier and write out the new identifier
	    
	    int newId = sqlite_last_insert_rowid(m_db);
	    writeIdentifier(album, newId);

	    // now copy the image properties from the old album to the new album
            
            execSql( QString("INSERT INTO Images (name, dirid, caption, datetime) "
                             "SELECT name, %1, caption, datetime FROM Images "
                             "WHERE dirid = %2;")
                     .arg(newId)
                     .arg(id) );

            // also copy the tags
            
            execSql( QString("INSERT INTO ImageTags (name, dirid, tagid) "
                             "SELECT name, %1, tagid FROM ImageTags "
                             "WHERE dirid = %2;")
                     .arg(newId)
                     .arg(id) );

	    id = newId;
        }
        else 
	{
            // no folder exists. that means this album has been renamed or moved
            // just update the url for the album
            execSql( QString("UPDATE Albums SET url = '%1' WHERE id = %2;")
                     .arg(escapeString(album->getURL()))
                     .arg(id) );

        }

	// Now update the properties of this album
        values.clear();
        execSql( QString("SELECT date, caption, collection "
                         "FROM Albums WHERE id = %1;")
                 .arg(id), &values );
        
	album->setID(id);
        album->setDate(QDate::fromString(values[0], Qt::ISODate), false);
        album->setCaption(values[1], false);
        album->setCollection(values[2], false);

        return true;
    }

    return false;
}

bool AlbumDB::execSql(const QString& sql, QStringList* const values,
                      const bool debug)
{
    if ( debug )
        kdDebug() << "SQL-query: " << sql << endl;

    if ( !m_db ) {
        kdWarning() << k_funcinfo << "SQLite pointer == NULL"
                    << endl;
        return false;
    }

    const char* tail;
    sqlite_vm* vm;
    char* errorStr;
    int error;
    
    //compile SQL program to virtual machine
    error = sqlite_compile( m_db, sql.local8Bit(), &tail, &vm, &errorStr );

    if ( error != SQLITE_OK ) {
        kdWarning() << k_funcinfo << "sqlite_compile error: "
                    << errorStr 
                    << " on query: " << sql << endl;
        sqlite_freemem( errorStr );
        return false;
    }

    int number;
    const char** value;
    const char** colName;
    //execute virtual machine by iterating over rows
    while ( true ) {
        error = sqlite_step( vm, &number, &value, &colName );
        if ( error == SQLITE_DONE || error == SQLITE_ERROR )
            break;
        //iterate over columns
        for ( int i = 0; values && i < number; i++ ) {
            *values << QString::fromLocal8Bit( value [i] );
        }
    }
    
    //deallocate vm resources
    sqlite_finalize( vm, &errorStr );

    if ( error != SQLITE_DONE ) {
        kdWarning() << k_funcinfo << "sqlite_step error: "
                    << errorStr
                    << " on query: " << sql << endl;
        return false;
    }

    return true;
}

QString AlbumDB::escapeString(QString str) const
{
    str.replace( "'", "''" );
    return str;
}

void AlbumDB::removeInvalidEntries()
{
    if (!m_db || !m_valid)
        return;
    
    beginTransaction();
    
    QStringList values;

    execSql( QString("SELECT url FROM Albums;"),
             &values );

    QString basePath(AlbumManager::instance()->getLibraryPath());

    for (QStringList::Iterator it = values.begin(); it != values.end(); ++it)
    {
        QFileInfo fi(basePath + *it);
        if (!fi.exists() || !fi.isDir()) {
            execSql( QString("DELETE FROM Albums WHERE url='%1';")
                     .arg(escapeString(*it)));
        }
    }
    
    commitTransaction();
}

QString AlbumDB::getItemCaption(PAlbum *album, const QString& name)
{
    QStringList values;

    execSql( QString("SELECT caption FROM Images "
                     "WHERE dirid='%1' AND name='%2';")
             .arg(album->getID())
             .arg(escapeString(name)),
             &values );

    if (!values.isEmpty())
        return values[0];
    else
        return QString::null;
}

QStringList AlbumDB::getItemTagNames(PAlbum *album, const QString& name)
{
    QStringList values;

    execSql( QString("SELECT Tags.name FROM ImageTags Join Tags "
                     "ON (ImageTags.dirid=%1 AND ImageTags.name='%2') "
                     "WHERE Tags.id=ImageTags.tagid "
                     "ORDER BY Tags.name;")
             .arg(album->getID())
             .arg(escapeString(name)),
             &values );

    return values;
}

IntList AlbumDB::getItemTagIDs(PAlbum *album, const QString& name)
{
    QStringList values;

    execSql( QString("SELECT tagid FROM ImageTags "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(album->getID())
             .arg(escapeString(name)),
             &values );

    IntList ids;
    
    if (values.isEmpty())
        return ids;
    
    for (QStringList::iterator it=values.begin(); it != values.end(); ++it)
    {
        ids << (*it).toInt();
    }
    return ids;
}

IntList AlbumDB::getItemCommonTagIDs(const IntList& dirIDList, const QStringList& nameList)
{
    IntList ids;
    
    if (dirIDList.isEmpty() || dirIDList.count() != nameList.count())
        return ids;

    QStringList values;

    QString sql = QString("SELECT tagid FROM ImageTags "
                          "WHERE (dirid=%1 AND name='%2')")
                  .arg(dirIDList.first())
                  .arg(escapeString(nameList.first()));

    IntList::const_iterator     diter = dirIDList.begin();
    QStringList::const_iterator niter = nameList.begin();

    ++diter;
    ++niter;

    while (diter != dirIDList.end())
    {
        sql += QString(" OR (dirid=%1 AND name='%2')")
               .arg(*diter)
               .arg(escapeString(*niter));
        ++diter;
        ++niter;
    }

    sql += QString(";");
    execSql( sql, &values );

    if (values.isEmpty())
        return ids;
    
    for (QStringList::iterator it=values.begin(); it != values.end(); ++it)
    {
        ids << (*it).toInt();
    }
    return ids;
}

void AlbumDB::setItemCaption(PAlbum *album, const QString& name, const QString& caption)
{
    QStringList values;

    // TODO: find a better way to do this

    int id = album->getID();
    
    execSql( QString("SELECT COUNT(name) FROM Images "
                     "WHERE dirid='%1' AND name='%2';")
             .arg(id)
             .arg(escapeString(name)), &values );

    if (values[0] == "0")
    {
        execSql( QString("INSERT INTO Images (name, dirid, caption) "
                         "VALUES('%1', '%2', '%3'); ")
                 .arg(escapeString(name))
                 .arg(id)
                 .arg(escapeString(caption)) );
    }
    else
    {
        execSql( QString("UPDATE Images SET caption='%1' "
                         "WHERE dirid='%1' AND name='%2';")
                 .arg(escapeString(caption))
                 .arg(id)
                 .arg(escapeString(name)) );
    }

}

void AlbumDB::setItemTag(PAlbum *album, const QString& name, TAlbum* tag)
{
    execSql( QString("REPLACE INTO ImageTags VALUES('%1', %2, %3);")
                 .arg(escapeString(name))
                 .arg(album->getID())
                 .arg(tag->getID()) );
}

void AlbumDB::removeItemTag(PAlbum *album, const QString& name, TAlbum* tag)
{
    execSql( QString("DELETE FROM ImageTags "
                     "WHERE dirid=%1 AND name='%2' AND tagid='%3';")
             .arg(album->getID())
             .arg(escapeString(name))
             .arg(tag->getID()) );
}

void AlbumDB::removeItemAllTags(PAlbum *album, const QString& name)
{
    execSql( QString("DELETE FROM ImageTags "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(album->getID())
             .arg(escapeString(name)) );
}

QStringList AlbumDB::getItemsURL(TAlbum* album)
{
    QStringList values;
    
    execSql( QString("SELECT Albums.url||'/'||ImageTags.name, Images.caption "
                     "FROM Albums JOIN ImageTags LEFT JOIN Images "
                     "ON ImageTags.tagid=%1 AND Albums.id=ImageTags.dirid "
                     "AND Images.dirid=ImageTags.dirid AND Images.name=ImageTags.name;")
             .arg(album->getID()), &values );

    QStringList urls;
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        urls.append(*it);
        it += 2;
    }
    
    return urls;
}

void AlbumDB::getItemsInTAlbum(TAlbum* album, QStringList& urls,
                               QValueList<int>& dirids)
{
    QStringList values;
    
    execSql( QString("SELECT Albums.url||'/'||ImageTags.name, ImageTags.dirid "
                     "FROM Albums JOIN ImageTags "
                     "ON ImageTags.tagid=%1 AND Albums.id=ImageTags.dirid;")
             .arg(album->getID()), &values );

    urls.clear();
    dirids.clear();
    
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        urls.append(*it++);
        dirids.append((*it++).toInt());
    }
}

void AlbumDB::scanTags(TAlbum *parent)
{
    int pid = parent->getID();
    
    QStringList values;
    execSql( QString("SELECT id, name, icon "
		     "FROM Tags where pid=%1 ORDER by name;").
             arg(pid), 
             &values );

    if (values.isEmpty())
	return;

    int     id;
    QString name, icon;
    
    for (QStringList::iterator it = values.begin(); it != values.end(); )
    {
	id       = (*it++).toInt();
	name     =  *it++;
	icon     =  *it++;

        TAlbum *album = new TAlbum(name, id);
        album->setPID(pid);
        album->setIcon(icon);
        album->setParent(parent);
        
        AlbumManager::instance()->insertTAlbum(album);
        scanTags(album);        
    }
}

void AlbumDB::copyItem(PAlbum *srcAlbum,  const QString& srcFile,
                       PAlbum *destAlbum, const QString& destFile)
{
    // first delete any stale database entries if any
    deleteItem(destAlbum, destFile);
    
    execSql( QString("INSERT INTO Images (dirid, name, caption, datetime) "
                     "SELECT %1, '%2', caption, datetime FROM Images "
                     "WHERE dirid=%3 AND name='%4';")
             .arg(destAlbum->getID())
             .arg(escapeString(destFile))
             .arg(srcAlbum->getID())
             .arg(escapeString(srcFile)) );

    execSql( QString("INSERT INTO ImageTags (dirid, name, tagid) "
                     "SELECT %1, '%2', tagid FROM ImageTags "
                     "WHERE dirid=%3 AND name='%4';")
             .arg(destAlbum->getID())
             .arg(escapeString(destFile))
             .arg(srcAlbum->getID())
             .arg(escapeString(srcFile)) );
}

void AlbumDB::moveItem(PAlbum *srcAlbum,  const QString& srcFile,
                       PAlbum *destAlbum, const QString& destFile)
{
    // first delete any stale database entries if any
    deleteItem(destAlbum, destFile);

    execSql( QString("UPDATE Images SET dirid=%1, name='%2' "
                     "WHERE dirid=%3 AND name='%4';")
             .arg(destAlbum->getID())
             .arg(escapeString(destFile))
             .arg(srcAlbum->getID())
             .arg(escapeString(srcFile)) );

    execSql( QString("UPDATE ImageTags SET dirid=%1, name='%2' "
                     "WHERE dirid=%3 AND name='%4';")
             .arg(destAlbum->getID())
             .arg(escapeString(destFile))
             .arg(srcAlbum->getID())
             .arg(escapeString(srcFile)) );
}

void AlbumDB::deleteItem(PAlbum *album, const QString& file)
{
    execSql( QString("DELETE FROM Images "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(album->getID())
             .arg(escapeString(file)) );

    execSql( QString("DELETE FROM ImageTags "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(album->getID())
             .arg(escapeString(file)) );
}

void AlbumDB::renameAlbum(Album *album, const QString& newName)
{
    if (album->type() == Album::TAG)
    {
        renameTAlbum(static_cast<TAlbum*>(album), newName);
    }
    else if (album->type() == Album::PHYSICAL)
    {
        renamePAlbum(static_cast<PAlbum*>(album), newName);
    }
}

void AlbumDB::renamePAlbum(PAlbum* album, const QString&)
{
    // album has url alredy set correctly

    QString url = escapeString(album->getURL());
    
    // first delete any stale albums left behind
    execSql( QString("DELETE FROM Albums WHERE url = '%1'")
             .arg(url) );

    execSql( QString("UPDATE Albums SET url = '%1' WHERE id = %2;")
             .arg(url)
             .arg(album->getID()) );
}

void AlbumDB::renameTAlbum(TAlbum* album, const QString& name)
{
    execSql( QString("UPDATE Tags SET name='%1' WHERE id=%2;")
             .arg(escapeString(name))
             .arg(album->getID()) );

    album->setTitle(name);
}

void AlbumDB::setIcon(TAlbum *album, const QString& icon)
{
    execSql( QString("UPDATE Tags SET icon='%1' WHERE id=%2;")
             .arg(escapeString(icon))
             .arg(album->getID()) );

    album->setIcon(icon);
}

void AlbumDB::setIcon(PAlbum *album, const QString& icon)
{
    execSql( QString("UPDATE Albums SET icon='%1' WHERE id=%2;")
             .arg(escapeString(icon))
             .arg(album->getID()) );
    
    album->setIcon(icon);
}

void AlbumDB::moveTAlbum(TAlbum *album, TAlbum *parent)
{
    execSql( QString("UPDATE Tags SET pid='%1' WHERE id=%2;")
             .arg(parent->getID())
             .arg(album->getID()) );
}

bool AlbumDB::addPAlbum(const QString& url, const QString& caption,
                        const QDate& date, const QString& collection)
{
    return execSql( QString("REPLACE INTO Albums (url, date, caption, collection) "
                            "VALUES('%1', '%2', '%3', '%4');")
                    .arg(escapeString(url))
                    .arg(date.toString(Qt::ISODate))
                    .arg(escapeString(caption))
                    .arg(escapeString(collection)));
}

bool AlbumDB::createTAlbum(TAlbum* parent, const QString& name,
                           const QString& icon)
{
    if (!parent)
        return false;

    if (!execSql( QString("INSERT INTO Tags (pid, name, icon) "
                          "VALUES( %1, '%2', '%3')")
                  .arg(parent->getID())
                  .arg(escapeString(name))
                  .arg(escapeString(icon)) ))
    {
        return false;
    }
        
    TAlbum *album = new TAlbum(name, sqlite_last_insert_rowid(m_db));
    album->setPID(parent->getID());
    album->setParent(parent);
    album->setIcon(icon);

    AlbumManager::instance()->insertTAlbum(album);
    return true;
}
