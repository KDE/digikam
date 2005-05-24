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

/** @file albumdb.cpp */

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>

#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmap.h>
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

    // Before 0.8 this table did not exist, so we have to check
    // and add it seperately.
    if (!values.contains("Settings"))
    {
        if (!execSql( QString("CREATE TABLE Settings "
                              "(keyword TEXT NOT NULL UNIQUE,"
                              "value TEXT);") ))
            return;
        else
            setSetting("DBVersion","1");
    }

    m_valid = true;
}

AlbumInfo::List AlbumDB::scanAlbums()
{
    AlbumInfo::List aList;

    QStringList values;
    execSql( QString("SELECT id, url, date, caption, collection, icon "
                     "FROM Albums;"), &values );

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        AlbumInfo info;
        info.type = AlbumInfo::PHYSICAL;
        
        info.id = (*it).toInt();
        ++it;
        info.url = *it;
        ++it;
        info.date = QDate::fromString(*it, Qt::ISODate);
        ++it;
        info.caption = *it;
        ++it;
        info.collection = *it;
        ++it;
        info.icon = *it;
        ++it;
        
        aList.append(info);        
    }
    
    return aList;
}

AlbumInfo::List AlbumDB::scanTags()
{
    AlbumInfo::List aList;

    QStringList values;
    execSql( QString("SELECT id, pid, name, icon "
                     "FROM Tags;"), &values );

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        AlbumInfo info;
        info.type = AlbumInfo::TAG;
        
        info.id   = (*it).toInt();
        ++it;
        info.pid  = (*it).toInt();
        ++it;
        info.name = *it;
        ++it;
        info.icon = *it;
        ++it;

        aList.append(info);        
    }
    
    return aList;
}

void AlbumDB::deleteAlbum(int albumID)
{
    execSql( QString("DELETE FROM Albums WHERE id='%1'")
             .arg(albumID) );
}

int AlbumDB::addTag(int parentTagID, const QString& name, const QString& icon)
{
    if (!m_db)
        return -1;

    if (!execSql( QString("INSERT INTO Tags (pid, name, icon) "
                          "VALUES( %1, '%2', '%3')")
                  .arg(parentTagID)
                  .arg(escapeString(name))
                  .arg(escapeString(icon)) ))
    {
        return -1;
    }

    return sqlite_last_insert_rowid(m_db);
}

void AlbumDB::deleteTag(int tagID)
{
    execSql( QString("DELETE FROM Tags WHERE id=%1")
                 .arg(tagID) );
}

void AlbumDB::beginTransaction()
{
    execSql( "BEGIN TRANSACTION;" );
}

void AlbumDB::commitTransaction()
{
    execSql( "COMMIT TRANSACTION;" );
}

int AlbumDB::addAlbum(const QString& url, const QString& caption,
                      const QDate& date, const QString& collection)
{
    if (!m_db)
        return -1;
    
    execSql( QString("REPLACE INTO Albums (url, date, caption, collection) "
                     "VALUES('%1', '%2', '%3', '%4');")
             .arg(escapeString(url))
             .arg(date.toString(Qt::ISODate))
             .arg(escapeString(caption))
             .arg(escapeString(collection)));

    int id = sqlite_last_insert_rowid(m_db);
    return id;
}


void AlbumDB::setAlbumCaption(int albumID, const QString& caption)
{
    execSql( QString("UPDATE Albums SET caption='%1' WHERE id=%2;")
             .arg(escapeString(caption))
             .arg(albumID) );
}

void AlbumDB::setAlbumCollection(int albumID, const QString& collection)
{
    execSql( QString("UPDATE Albums SET collection='%1' WHERE id=%2;")
             .arg(escapeString(collection))
             .arg(albumID) );
}

void AlbumDB::setAlbumDate(int albumID, const QDate& date)
{
    execSql( QString("UPDATE Albums SET date='%1' WHERE id = %2;")
             .arg(date.toString(Qt::ISODate))
             .arg(albumID) );
}

void AlbumDB::setAlbumIcon(int albumID, const QString& icon)
{
    execSql( QString("UPDATE Albums SET icon='%1' WHERE id=%2;")
             .arg(escapeString(icon))
             .arg(albumID) );
}

void AlbumDB::setTagIcon(int tagID, const QString& icon)
{
    execSql( QString("UPDATE Tags SET icon='%1' WHERE id=%2;")
             .arg(escapeString(icon))
             .arg(tagID) );
}

void AlbumDB::setSetting(const QString& keyword,
                         const QString& value )
{
    execSql( QString("REPLACE into Settings VALUES ('%1','%2');")
            .arg( escapeString(keyword) )
            .arg( escapeString(value) ));
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

QDateTime AlbumDB::getItemDate(PAlbum *album, const QString& name)
{
    QStringList values;

    execSql( QString("SELECT datetime FROM Images "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(album->getID())
             .arg(escapeString(name)),
             &values );

    if (!values.isEmpty())
        return QDateTime();
    else
        return QDateTime::fromString(values[0], Qt::ISODate);
}

QString AlbumDB::getSetting(const QString& keyword)
{
    QStringList values;
    execSql( QString("SELECT value FROM Settings "
                     "WHERE keyword='%1';")
            .arg(escapeString(keyword)),
            &values );

    if (values.isEmpty())
        return QString::null;
    else
        return values[0];
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

QStringList AlbumDB::getItemNamesInAlbum(int albumID)
{
    QStringList values;
    execSql( QString("SELECT Images.name "
                     "FROM Images "
                     "WHERE Images.dirid=%1")
             .arg(albumID), &values );

    return values;
}

QStringList AlbumDB::getAllItemURLsWithoutDate()
{
    QStringList values;
    execSql( QString("SELECT Albums.url||'/'||Images.name "
                     "FROM Images, Albums "
                     "WHERE Images.dirid=Albums.Id "
                     "AND (Images.datetime is null or "
                     "     Images.datetime == '');"),
             &values );

    QString libraryPath = AlbumManager::instance()->getLibraryPath() + "/";
    for (QStringList::iterator it = values.begin(); it != values.end();
         ++it)
    {
        *it = libraryPath + *it;
    }
    
    return values;
}

int AlbumDB::getOrCreateAlbumId(const QString& folder)
{
    QStringList values;
    execSql( QString("SELECT id FROM Albums WHERE url ='%1';")
            .arg( escapeString(folder) ), &values);

    int albumID;
    if (values.isEmpty())
    {
        execSql( QString ("INSERT INTO Albums (url, date) "
                "VALUES ('%1','%2')")
                .arg(escapeString(folder))
                .arg(QDateTime::currentDateTime().toString(Qt::ISODate)) );
        albumID = sqlite_last_insert_rowid(m_db);
    } else 
        albumID = values[0].toInt();

    return albumID;
}

bool AlbumDB::setItemDateComment(int albumID, 
                                 const QString& name,
                                 const QDateTime& datetime, 
                                 const QString& comment)
{
    execSql ( QString ("REPLACE INTO Images "
                       "( caption , datetime, name, dirid ) "
                       " VALUES ('%1','%2','%3',%4) " )
              .arg(escapeString(comment))
              .arg(datetime.toString(Qt::ISODate))
              .arg(escapeString(name))
              .arg(albumID) );
    
    return true; 
}

bool AlbumDB::setItemDate(int albumID, const QString& name,
                          const QDateTime& datetime)
{
    execSql ( QString ("UPDATE Images SET datetime='%1'"
                       "WHERE dirid='%1' AND name='%2';")
              .arg(datetime.toString(Qt::ISODate))
              .arg(albumID)
              .arg(escapeString(name)) );

    return true; 
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

QDate AlbumDB::getAlbumAverageDate(PAlbum *album)
{
    QStringList values;
    execSql( QString("SELECT datetime FROM Images WHERE dirid=%1")
            .arg( album->getID() ), &values);

    int differenceInSecs = 0;
    int amountOfImages = 0;
    QDateTime baseDateTime;

    for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
    {
        QDateTime itemDateTime = QDateTime::fromString( *it, Qt::ISODate );
        if (itemDateTime.isValid())
        {
            ++amountOfImages;
            if ( baseDateTime.isNull() )
                baseDateTime=itemDateTime;
            else
                differenceInSecs += itemDateTime.secsTo( baseDateTime );
        }
    }

    if ( amountOfImages > 0 )
    {
        QDateTime averageDateTime;
        averageDateTime.setTime_t( baseDateTime.toTime_t() -
                (int)( differenceInSecs/amountOfImages ) );
        return ( averageDateTime.date() );
    }
    else
        return QDate();
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
    deleteItem(album->getID(), file);
}

void AlbumDB::deleteItem(int albumID, const QString& file)
{
    execSql( QString("DELETE FROM Images "
            "WHERE dirid=%1 AND name='%2';")
            .arg(albumID)
            .arg(escapeString(file)) );

    execSql( QString("DELETE FROM ImageTags "
            "WHERE dirid=%1 AND name='%2';")
            .arg(albumID)
            .arg(escapeString(file)) );
}

void AlbumDB::setAlbumURL(int albumID, const QString& url)
{
    QString u = escapeString(u);
    
    // first delete any stale albums left behind
    execSql( QString("DELETE FROM Albums WHERE url = '%1'")
             .arg(u) );

    // now update the album url
    execSql( QString("UPDATE Albums SET url = '%1' WHERE id = %2;")
             .arg(url)
             .arg(albumID) );
}

void AlbumDB::setTagName(int tagID, const QString& name)
{
    execSql( QString("UPDATE Tags SET name='%1' WHERE id=%2;")
             .arg(escapeString(name))
             .arg(tagID) );
}

void AlbumDB::moveTAlbum(TAlbum *album, TAlbum *parent)
{
    execSql( QString("UPDATE Tags SET pid='%1' WHERE id=%2;")
             .arg(parent->getID())
             .arg(album->getID()) );
}

