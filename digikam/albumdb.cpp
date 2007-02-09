/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2004-06-18
 * Description : database album interface.
 *
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
 * Copyright 2006 by Marcel Wiesweg
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

/** @file albumdb.cpp */

// C Ansi includes.

extern "C"
{
#include <sqlite3.h>
#include <sys/time.h>
}

// C++ includes.

#include <cstdio>
#include <cstdlib>
#include <ctime>

// Qt includes.

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "albummanager.h"
#include "album.h"
#include "albumdb.h"

namespace Digikam
{

typedef struct sqlite3_stmt sqlite3_stmt;
typedef struct sqlite3 sqleet3;            // hehe.

class AlbumDBPriv
{

public:

    AlbumDBPriv()
    {
        valid    = false;
        dataBase = 0;
    }

    bool     valid;

    sqleet3 *dataBase;

    IntList  recentlyAssignedTags;
};

AlbumDB::AlbumDB()
{
    d = new AlbumDBPriv;
}

AlbumDB::~AlbumDB()
{
    if (d->dataBase)
    {
        sqlite3_close(d->dataBase);
    }

    delete d;
}

bool AlbumDB::isValid() const
{
    return d->valid; 
}

void AlbumDB::setDBPath(const QString& path)
{
    if (d->dataBase)
    {
        sqlite3_close(d->dataBase);
        d->dataBase = 0;
    }

    d->valid = false;

    sqlite3_open(QFile::encodeName(path), &d->dataBase);
    if (d->dataBase == 0)
    {
        DWarning() << "Cannot open database: "
                    << sqlite3_errmsg(d->dataBase)
                    << endl;
    }
    else
    {
        initDB();
    }
}

void AlbumDB::initDB()
{
    d->valid = false;

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
        if (!execSql( QString("CREATE TABLE Albums\n"
                              " (id INTEGER PRIMARY KEY,\n"
                              "  url TEXT NOT NULL UNIQUE,\n"
                              "  date DATE NOT NULL,\n"
                              "  caption TEXT,\n"
                              "  collection TEXT,\n"
                              "  icon INTEGER);") ))
        {
            return;
        }

        if (!execSql( QString("CREATE TABLE Tags\n"
                              " (id INTEGER PRIMARY KEY,\n"
                              "  pid INTEGER,\n"
                              "  name TEXT NOT NULL,\n"
                              "  icon INTEGER,\n"
                              "  iconkde TEXT,\n"
                              "  UNIQUE (name, pid));") ))
        {
            return;
        }

        if (!execSql( QString("CREATE TABLE TagsTree\n"
                              " (id INTEGER NOT NULL,\n"
                              "  pid INTEGER NOT NULL,\n"
                              "  UNIQUE (id, pid));") ))
        {
            return;
        }

        if (!execSql( QString("CREATE TABLE Images\n"
                              " (id INTEGER PRIMARY KEY,\n"
                              "  name TEXT NOT NULL,\n"
                              "  dirid INTEGER NOT NULL,\n"
                              "  caption TEXT,\n"
                              "  datetime DATETIME,\n"
                              "  UNIQUE (name, dirid));") ))
        {
            return;
        }


        if (!execSql( QString("CREATE TABLE ImageTags\n"
                              " (imageid INTEGER NOT NULL,\n"
                              "  tagid INTEGER NOT NULL,\n"
                              "  UNIQUE (imageid, tagid));") ))
        {
            return;
        }

        if (!execSql( QString("CREATE TABLE ImageProperties\n"
                              " (imageid  INTEGER NOT NULL,\n"
                              "  property TEXT    NOT NULL,\n"
                              "  value    TEXT    NOT NULL,\n"
                              "  UNIQUE (imageid, property));") ))
        {
            return;
        }

        if ( !execSql( QString( "CREATE TABLE Searches  \n"
                                " (id INTEGER PRIMARY KEY, \n"
                                "  name TEXT NOT NULL UNIQUE, \n"
                                "  url  TEXT NOT NULL);" ) ) )
        {
            return;
        }

        if (!execSql( QString("CREATE TABLE Settings         \n"
                              "(keyword TEXT NOT NULL UNIQUE,\n"
                              " value TEXT);") ))
            return;
        else
            setSetting("DBVersion","1");

        // TODO: see which more indices are needed
        // create indices
        execSql("CREATE INDEX dir_index ON Images    (dirid);");
        execSql("CREATE INDEX tag_index ON ImageTags (tagid);");

        // create triggers

        // trigger: delete from Images/ImageTags/ImageProperties
        // if Album has been deleted
        execSql("CREATE TRIGGER delete_album DELETE ON Albums\n"
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
        execSql("CREATE TRIGGER delete_image DELETE ON Images\n"
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
        execSql("CREATE TRIGGER delete_tag DELETE ON Tags\n"
                "BEGIN\n"
                "  DELETE FROM ImageTags WHERE tagid=OLD.id;\n"
                "END;");

        // trigger: insert into TagsTree if Tag has been added
        execSql("CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags\n"
                "BEGIN\n"
                "  INSERT INTO TagsTree\n"
                "    SELECT NEW.id, NEW.pid\n"
                "    UNION\n"
                "    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;\n"
                "END;");

        // trigger: delete from TagsTree if Tag has been deleted
        execSql("CREATE TRIGGER delete_tagstree DELETE ON Tags\n"
                "BEGIN\n"
                " DELETE FROM Tags\n"
                "   WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
                " DELETE FROM TagsTree\n"
                "   WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
                " DELETE FROM TagsTree\n"
                "    WHERE id=OLD.id;\n"
                "END;");

        // trigger: delete from TagsTree if Tag has been deleted
        execSql("CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags\n"
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
    }

    d->valid = true;
}

AlbumInfo::List AlbumDB::scanAlbums()
{
    AlbumInfo::List aList;

    QString basePath(AlbumManager::instance()->getLibraryPath());

    QStringList values;
    execSql( "SELECT A.id, A.url, A.date, A.caption, A.collection, B.url, I.name \n "
             "FROM Albums AS A \n "
             "  LEFT OUTER JOIN Images AS I ON A.icon=I.id \n"
             "  LEFT OUTER JOIN Albums AS B ON B.id=I.dirid;", &values);

    QString iconAlbumUrl, iconName;

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        AlbumInfo info;

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
        iconAlbumUrl = *it;
        ++it;
        iconName = *it;
        ++it;

        if (!iconName.isEmpty())
        {
            info.icon = basePath + iconAlbumUrl + '/' + iconName;
        }

        aList.append(info);
    }

    return aList;
}

TagInfo::List AlbumDB::scanTags()
{
    TagInfo::List tList;

    QString basePath(AlbumManager::instance()->getLibraryPath());

    QStringList values;
    execSql( "SELECT T.id, T.pid, T.name, A.url, I.name, T.iconkde \n "
             "FROM Tags AS T LEFT OUTER JOIN Images AS I ON I.id=T.icon \n "
             "  LEFT OUTER JOIN Albums AS A ON A.id=I.dirid; ", &values );

    QString iconName, iconKDE, albumURL;

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        TagInfo info;

        info.id     = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = *it;
        ++it;
        albumURL    = *it;
        ++it;
        iconName    = *it;
        ++it;
        iconKDE     = *it;
        ++it;

        if ( albumURL.isEmpty() )
        {
            info.icon = iconKDE;
        }
        else
        {
            info.icon = basePath + albumURL + '/' + iconName;
        }

        tList.append(info);
    }

    return tList;
}

SearchInfo::List AlbumDB::scanSearches()
{
    SearchInfo::List searchList;    

    QStringList values;
    execSql( "SELECT id, name, url FROM Searches;", &values);

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        SearchInfo info;

        info.id   = (*it).toInt();
        ++it;
        info.name = (*it);
        ++it;
        info.url  = (*it);
        ++it;

        searchList.append(info);        
    }

    return searchList;
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
    if (!d->dataBase)
        return -1;

    execSql( QString("REPLACE INTO Albums (url, date, caption, collection) "
                     "VALUES('%1', '%2', '%3', '%4');")
             .arg(escapeString(url),
                  date.toString(Qt::ISODate),
                  escapeString(caption),
                  escapeString(collection)));

    int id = sqlite3_last_insert_rowid(d->dataBase);
    return id;
}

void AlbumDB::setAlbumCaption(int albumID, const QString& caption)
{
    execSql( QString("UPDATE Albums SET caption='%1' WHERE id=%2;")
             .arg(escapeString(caption),
                  QString::number(albumID) ));
}

void AlbumDB::setAlbumCollection(int albumID, const QString& collection)
{
    execSql( QString("UPDATE Albums SET collection='%1' WHERE id=%2;")
             .arg(escapeString(collection),
                  QString::number(albumID)) );
}

void AlbumDB::setAlbumDate(int albumID, const QDate& date)
{
    execSql( QString("UPDATE Albums SET date='%1' WHERE id=%2;")
             .arg(date.toString(Qt::ISODate))
             .arg(albumID) );
}

void AlbumDB::setAlbumIcon(int albumID, Q_LLONG iconID)
{
    execSql( QString("UPDATE Albums SET icon=%1 WHERE id=%2;")
             .arg(iconID)
             .arg(albumID) );
}

    
QString AlbumDB::getAlbumIcon(int albumID)
{
    QStringList values;
    execSql( QString("SELECT B.url, I.name \n "
                     "FROM Albums AS A \n "
                     "  LEFT OUTER JOIN Images AS I ON I.id=A.icon \n "
                     "  LEFT OUTER JOIN Albums AS B ON B.id=I.dirid \n "
                     "WHERE A.id=%1;")
             .arg(albumID), &values );
    if (values.isEmpty())
        return QString();

    QStringList::iterator it = values.begin();
    QString url  = *it;
    ++it;
    QString icon = *it;
    if (icon.isEmpty())
        return QString();

    QString basePath(AlbumManager::instance()->getLibraryPath());
    basePath += url;
    basePath += '/' + icon;

    return basePath;
}

void AlbumDB::deleteAlbum(int albumID)
{
    execSql( QString("DELETE FROM Albums WHERE id=%1")
             .arg(albumID) );
}

int AlbumDB::addTag(int parentTagID, const QString& name, const QString& iconKDE,
                    Q_LLONG iconID)
{
    if (!d->dataBase)
        return -1;

    if (!execSql( QString("INSERT INTO Tags (pid, name) "
                          "VALUES( %1, '%2')")
                  .arg(parentTagID)
                  .arg(escapeString(name))))
    {
        return -1;
    }

    int id = sqlite3_last_insert_rowid(d->dataBase);

    if (!iconKDE.isEmpty())
    {
        execSql( QString("UPDATE Tags SET iconkde='%1' WHERE id=%2;")
                 .arg(escapeString(iconKDE),
                      QString::number(id)));
    }
    else
    {
        execSql( QString("UPDATE Tags SET icon=%1 WHERE id=%2;")
                 .arg(iconID)
                 .arg(id));
    }
    
    return id;
}

void AlbumDB::deleteTag(int tagID)
{
    execSql( QString("DELETE FROM Tags WHERE id=%1")
                 .arg(tagID) );
}

void AlbumDB::setTagIcon(int tagID, const QString& iconKDE, Q_LLONG iconID)
{
    if (!iconKDE.isEmpty())
    {
        execSql( QString("UPDATE Tags SET iconkde='%1', icon=0 WHERE id=%2;")
                 .arg(escapeString(iconKDE), 
                      QString::number(tagID)));
    }
    else
    {
        execSql( QString("UPDATE Tags SET icon=%1 WHERE id=%2;")
                 .arg(iconID)
                 .arg(tagID));
    }
}

QString AlbumDB::getTagIcon(int tagID)
{
    QStringList values;
    execSql( QString("SELECT A.url, I.name, T.iconkde \n "
                     "FROM Tags AS T \n "
                     "  LEFT OUTER JOIN Images AS I ON I.id=T.icon \n "
                     "  LEFT OUTER JOIN Albums AS A ON A.id=I.dirid \n "
                     "WHERE T.id=%1;")
             .arg(tagID), &values );

    if (values.isEmpty())
        return QString();
    
    QString basePath(AlbumManager::instance()->getLibraryPath());

    QString iconName, iconKDE, albumURL, icon;

    QStringList::iterator it = values.begin();

    albumURL    = *it;
    ++it;
    iconName    = *it;
    ++it;
    iconKDE     = *it;
    ++it;

    if ( albumURL.isEmpty() )
    {
        icon = iconKDE;
    }
    else
    {
        icon = basePath + albumURL + '/' + iconName;
    }

    return icon;
}

void AlbumDB::setTagParentID(int tagID, int newParentTagID)
{
    execSql( QString("UPDATE Tags SET pid=%1 WHERE id=%2;")
             .arg(newParentTagID)
             .arg(tagID) );
}

int AlbumDB::addSearch(const QString& name, const KURL& url)
{
    if (!d->dataBase)
    return -1;

    QString str("INSERT INTO Searches (name, url) \n"
                "VALUES('$$@@$$', '$$##$$');");
    str.replace("$$@@$$", escapeString(name));
    str.replace("$$##$$", escapeString(url.url()));
    
    if (!execSql(str))
    {
    return -1;
    }

    return sqlite3_last_insert_rowid(d->dataBase);
}

void AlbumDB::updateSearch(int searchID, const QString& name,
               const KURL& url)
{
    QString str = QString("UPDATE Searches SET name='$$@@$$', url='$$##$$' \n"
                          "WHERE id=%1")
                  .arg(searchID);
    str.replace("$$@@$$", escapeString(name));
    str.replace("$$##$$", escapeString(url.url()));

    execSql(str);
}

void AlbumDB::deleteSearch(int searchID)
{
    execSql( QString("DELETE FROM Searches WHERE id=%1")
             .arg(searchID) );
}

void AlbumDB::setSetting(const QString& keyword,
                         const QString& value )
{
    execSql( QString("REPLACE into Settings VALUES ('%1','%2');")
             .arg(escapeString(keyword),
                  escapeString(value) ));
}

QString AlbumDB::getSetting(const QString& keyword)
{
    QStringList values;
    execSql( QString("SELECT value FROM Settings "
                     "WHERE keyword='%1';")
             .arg(escapeString(keyword)), &values );

    if (values.isEmpty())
        return QString::null;
    else
        return values[0];
}

bool AlbumDB::execSql(const QString& sql, QStringList* const values,
                      const bool debug)
{
    if ( debug )
        DDebug() << "SQL-query: " << sql << endl;

    if ( !d->dataBase )
    {
        DWarning() << k_funcinfo << "SQLite pointer == NULL"
                    << endl;
        return false;
    }

    const char*   tail;
    sqlite3_stmt* stmt;
    int           error;

    //compile SQL program to virtual machine
    error = sqlite3_prepare(d->dataBase, sql.utf8(), -1, &stmt, &tail);
    if ( error != SQLITE_OK )
    {
        DWarning() << k_funcinfo
                    << "sqlite_compile error: "
                    << sqlite3_errmsg(d->dataBase)
                    << " on query: "
                    << sql << endl;
        return false;
    }

    int cols = sqlite3_column_count(stmt);

    while ( true )
    {
        error = sqlite3_step( stmt );

        if ( error == SQLITE_DONE || error == SQLITE_ERROR )
            break;

        //iterate over columns
        for ( int i = 0; values && i < cols; i++ )
        {
            *values << QString::fromUtf8( (const char*)sqlite3_column_text( stmt, i ) );
        }
    }

    sqlite3_finalize( stmt );

    if ( error != SQLITE_DONE )
    {
        DWarning() << "sqlite_step error: "
                    << sqlite3_errmsg( d->dataBase )
                    << " on query: "
                    << sql << endl;
        return false;
    }

    return true;
}

QString AlbumDB::escapeString(QString str) const
{
    str.replace( "'", "''" );
    return str;
}

QString AlbumDB::getItemCaption(Q_LLONG imageID)
{
    QStringList values;

    execSql( QString("SELECT caption FROM Images "
                     "WHERE id=%1;")
             .arg(imageID),
             &values );

    if (!values.isEmpty())
        return values[0];
    else
        return QString::null;
}

QString AlbumDB::getItemCaption(int albumID, const QString& name)
{
    QStringList values;

    execSql( QString("SELECT caption FROM Images "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(albumID)
             .arg(escapeString(name)),
             &values );

    if (!values.isEmpty())
        return values[0];
    else
        return QString::null;
}

QDateTime AlbumDB::getItemDate(Q_LLONG imageID)
{
    QStringList values;

    execSql( QString("SELECT datetime FROM Images "
                     "WHERE id=%1;")
             .arg(imageID),
             &values );

    if (values.isEmpty())
        return QDateTime();
    else
        return QDateTime::fromString(values[0], Qt::ISODate);
}

QDateTime AlbumDB::getItemDate(int albumID, const QString& name)
{
    QStringList values;

    execSql( QString("SELECT datetime FROM Images "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(albumID)
             .arg(escapeString(name)),
             &values );

    if (values.isEmpty())
        return QDateTime();
    else
        return QDateTime::fromString(values[0], Qt::ISODate);
}

Q_LLONG AlbumDB::getImageId(int albumID, const QString& name)
{
    QStringList values;

    execSql( QString("SELECT id FROM Images "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(albumID)
             .arg(escapeString(name)),
             &values );

    if (values.isEmpty())
        return -1;
    else
        return (values[0]).toLongLong();
}

QStringList AlbumDB::getItemTagNames(Q_LLONG imageID)
{
    QStringList values;

    execSql( QString("SELECT name FROM Tags \n "
                     "WHERE id IN (SELECT tagid FROM ImageTags \n "
                     "             WHERE imageid=%1) \n "
                     "ORDER BY name;")
             .arg(imageID),
             &values );
    
    return values;
}

IntList AlbumDB::getItemTagIDs(Q_LLONG imageID)
{
    QStringList values;

    execSql( QString("SELECT tagid FROM ImageTags \n "
                     "WHERE imageID=%1;")
             .arg(imageID),
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

bool AlbumDB::hasTags(const LLongList& imageIDList)
{
    IntList ids;

    if (imageIDList.isEmpty())
        return false;

    QStringList values;

    QString sql = QString("SELECT count(tagid) FROM ImageTags "
            "WHERE imageid=%1 ")
            .arg(imageIDList.first());

    LLongList::const_iterator iter = imageIDList.begin();
    ++iter;

    while (iter != imageIDList.end())
    {
        sql += QString(" OR imageid=%2 ")
                .arg(*iter);
        ++iter;
    }

    sql += QString(";");
    execSql( sql, &values );

    if (values[0] == "0")
        return false;
    else
        return true;
}

IntList AlbumDB::getItemCommonTagIDs(const LLongList& imageIDList)
{
    IntList ids;

    if (imageIDList.isEmpty())
        return ids;

    QStringList values;

    QString sql = QString("SELECT DISTINCT tagid FROM ImageTags "
                          "WHERE imageid=%1 ")
                  .arg(imageIDList.first());

    LLongList::const_iterator iter = imageIDList.begin();
    ++iter;

    while (iter != imageIDList.end())
    {
        sql += QString(" OR imageid=%2 ")
               .arg(*iter);
        ++iter;
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

void AlbumDB::setItemCaption(Q_LLONG imageID,const QString& caption)
{
    QStringList values;

    execSql( QString("UPDATE Images SET caption='%1' "
                     "WHERE id=%2;")
             .arg(escapeString(caption),
                  QString::number(imageID) ));
}

void AlbumDB::setItemCaption(int albumID, const QString& name, const QString& caption)
{
    QStringList values;

    execSql( QString("UPDATE Images SET caption='%1' "
                     "WHERE dirid=%2 AND name='%3';")
             .arg(escapeString(caption),
                  QString::number(albumID),
                  escapeString(name)) );
}

void AlbumDB::addItemTag(Q_LLONG imageID, int tagID)
{
    execSql( QString("REPLACE INTO ImageTags (imageid, tagid) "
                     "VALUES(%1, %2);")
                 .arg(imageID)
                 .arg(tagID) );

    if (!d->recentlyAssignedTags.contains(tagID))
    {
        d->recentlyAssignedTags.push_front(tagID);
        if (d->recentlyAssignedTags.size() > 10)
            d->recentlyAssignedTags.pop_back();
    }
}

void AlbumDB::addItemTag(int albumID, const QString& name, int tagID)
{
    execSql( QString("REPLACE INTO ImageTags (imageid, tagid) \n "
                     "(SELECT id, %1 FROM Images \n "
                     " WHERE dirid=%2 AND name='%3');")
             .arg(tagID)
             .arg(albumID)
             .arg(escapeString(name)) );
}

IntList AlbumDB::getRecentlyAssignedTags() const
{
    return d->recentlyAssignedTags;    
}

void AlbumDB::removeItemTag(Q_LLONG imageID, int tagID)
{
    execSql( QString("DELETE FROM ImageTags "
                     "WHERE imageID=%1 AND tagid=%2;")
             .arg(imageID)
             .arg(tagID) );
}

void AlbumDB::removeItemAllTags(Q_LLONG imageID)
{
    execSql( QString("DELETE FROM ImageTags "
                     "WHERE imageID=%1;")
             .arg(imageID) );
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

    QString libraryPath = AlbumManager::instance()->getLibraryPath() + '/';
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
                 .arg(escapeString(folder),
                      QDateTime::currentDateTime().toString(Qt::ISODate)) );
        albumID = sqlite3_last_insert_rowid(d->dataBase);
    } else
        albumID = values[0].toInt();

    return albumID;
}

Q_LLONG AlbumDB::addItem(int albumID,
                         const QString& name,
                         const QDateTime& datetime,
                         const QString& comment,
                         int rating,
                         const QStringList &keywordsList)
{
    execSql ( QString ("REPLACE INTO Images "
                       "( caption , datetime, name, dirid ) "
                       " VALUES ('%1','%2','%3',%4) " )
              .arg(escapeString(comment),
                   datetime.toString(Qt::ISODate),
                   escapeString(name),
                   QString::number(albumID)) );

    Q_LLONG item = sqlite3_last_insert_rowid(d->dataBase);

    // Set Rating value to item in database.

    if ( item != -1 && rating != -1 )
        setItemRating(item, rating);

    // Set existing tags in database or create new tags if not exist.

    if ( item != -1 && !keywordsList.isEmpty() )
    {
        IntList tagIDs = getTagsFromTagPaths(keywordsList);
        for (IntList::iterator it = tagIDs.begin(); it != tagIDs.end(); ++it)
        {
            addItemTag(item, *it);
        }
    }

    return item;
}

IntList AlbumDB::getTagsFromTagPaths(const QStringList &keywordsList, bool create)
{
    if (keywordsList.isEmpty())
        return IntList();

    IntList tagIDs;

    QStringList keywordsList2Create;

    // Create a list of the tags currently in database

    TagInfo::List currentTagsList;

    QStringList values;
    execSql( "SELECT id, pid, name FROM Tags;", &values );

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        TagInfo info;

        info.id   = (*it).toInt();
        ++it;
        info.pid  = (*it).toInt();
        ++it;
        info.name = *it;
        ++it;
        currentTagsList.append(info);
    }

    // For every tag in keywordsList, scan taglist to check if tag already exists.

    for (QStringList::const_iterator kwd = keywordsList.begin();
        kwd != keywordsList.end(); ++kwd )
    {
        // split full tag "url" into list of single tag names
        QStringList tagHierarchy = QStringList::split('/', *kwd);
        if (tagHierarchy.isEmpty())
            continue;

        // last entry in list is the actual tag name
        bool foundTag   = false;
        QString tagName = tagHierarchy.back();
        tagHierarchy.pop_back();

        for (TagInfo::List::iterator tag = currentTagsList.begin();
            tag != currentTagsList.end(); ++tag )
        {
            // There might be multiple tags with the same name, but in different
            // hierarchies. We must check them all until we find the correct hierarchy
            if ((*tag).name == tagName)
            {
                int parentID = (*tag).pid;

                // Check hierarchy, from bottom to top
                bool foundParentTag                 = true;
                QStringList::iterator parentTagName = tagHierarchy.end();

                while (foundParentTag && parentTagName != tagHierarchy.begin())
                {
                    --parentTagName;

                    foundParentTag = false;

                    for (TagInfo::List::iterator parentTag = currentTagsList.begin();
                        parentTag != currentTagsList.end(); ++parentTag )
                    {
                        // check if name is the same, and if ID is identical
                        // to the parent ID we got from the child tag
                        if ( (*parentTag).id == parentID &&
                            (*parentTag).name == (*parentTagName) )
                        {
                            parentID       = (*parentTag).pid;
                            foundParentTag = true;
                            break;
                        }
                    }

                    // If we traversed the list without a match,
                    // foundParentTag will be false, the while loop breaks.
                }

                // If we managed to traverse the full hierarchy,
                // we have our tag.
                if (foundParentTag)
                {
                    // add to result list
                    tagIDs.append((*tag).id);
                    foundTag = true;
                    break;
                }
            }
        }

        if (!foundTag)
            keywordsList2Create.append(*kwd);
    }

    // If tags do not exist in database, create them.

    if (create && !keywordsList2Create.isEmpty())
    {
        for (QStringList::iterator kwd = keywordsList2Create.begin();
            kwd != keywordsList2Create.end(); ++kwd )
        {
            // split full tag "url" into list of single tag names
            QStringList tagHierarchy = QStringList::split('/', *kwd);

            if (tagHierarchy.isEmpty())
                continue;

            int  parentTagID      = 0;
            int  tagID            = 0;
            bool parentTagExisted = true;

            // Traverse hierarchy from top to bottom
            for (QStringList::iterator tagName = tagHierarchy.begin();
                tagName != tagHierarchy.end(); ++tagName)
            {
                tagID = 0;

                // if the parent tag did not exist, we need not check if the child exists
                if (parentTagExisted)
                {
                    for (TagInfo::List::iterator tag = currentTagsList.begin();
                        tag != currentTagsList.end(); ++tag )
                    {
                        // find the tag with tag name according to tagHierarchy,
                        // and parent ID identical to the ID of the tag we found in
                        // the previous run.
                        if ((*tag).name == (*tagName) && (*tag).pid == parentTagID)
                        {
                            tagID = (*tag).id;
                            break;
                        }
                    }
                }

                if (tagID != 0)
                {
                    // tag already found in DB
                    parentTagID = tagID;
                    continue;
                }

                // Tag does not yet exist in DB, add it
                tagID = addTag(parentTagID, (*tagName), QString::null, 0);

                if (tagID == -1)
                {
                    // Something is wrong in database. Abort.
                    break;
                }

                // append to our list of existing tags (for following keywords)
                TagInfo info;
                info.id   = tagID;
                info.pid  = parentTagID;
                info.name = (*tagName);
                currentTagsList.append(info);

                parentTagID      = tagID;
                parentTagExisted = false;
            }

            // add to result list
            tagIDs.append(tagID);
        }
    }

    return tagIDs;
}

int AlbumDB::getItemAlbum(Q_LLONG imageID)
{
    QStringList values;

    execSql ( QString ("SELECT dirid FROM Images "
                       "WHERE id=%1;")
              .arg(imageID),
              &values);

    if (!values.isEmpty())
        return values.first().toInt();
    else
        return 1;
}

QString AlbumDB::getItemName(Q_LLONG imageID)
{
    QStringList values;

    execSql ( QString ("SELECT name FROM Images "
                       "WHERE id=%1;")
              .arg(imageID),
              &values);

    if (!values.isEmpty())
        return values.first();
    else
        return QString();
}

bool AlbumDB::setItemDate(Q_LLONG imageID,
                          const QDateTime& datetime)
{
    execSql ( QString ("UPDATE Images SET datetime='%1'"
                       "WHERE id=%2;")
              .arg(datetime.toString(Qt::ISODate),
                   QString::number(imageID)) );

    return true;
}

bool AlbumDB::setItemDate(int albumID, const QString& name,
                          const QDateTime& datetime)
{
    execSql ( QString ("UPDATE Images SET datetime='%1'"
                       "WHERE dirid=%2 AND name='%3';")
              .arg(datetime.toString(Qt::ISODate),
                   QString::number(albumID),
                   escapeString(name)) );

    return true;
}

void AlbumDB::setItemRating(Q_LLONG imageID, int rating)
{
    execSql ( QString ("REPLACE INTO ImageProperties "
                       "(imageid, property, value) "
                       "VALUES(%1, '%2', '%3');")
              .arg(imageID)
              .arg("Rating")
              .arg(rating) );
}

int AlbumDB::getItemRating(Q_LLONG imageID)
{
    QStringList values;

    execSql( QString("SELECT value FROM ImageProperties "
                     "WHERE imageid=%1 and property='%2';")
             .arg(imageID)
             .arg("Rating"),
             &values);

    if (!values.isEmpty())
        return values[0].toInt();
    else
        return 0;
}

QStringList AlbumDB::getItemURLsInAlbum(int albumID)
{
    QStringList values;

    QString basePath(AlbumManager::instance()->getLibraryPath());

    execSql( QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                     "WHERE Albums.id=%1 AND Albums.id=Images.dirid;")
             .arg(albumID), &values );

    for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
    {
        *it = basePath + *it;
    }

    return values;
}

QStringList AlbumDB::getItemURLsInTag(int tagID)
{
    QStringList values;

    QString basePath(AlbumManager::instance()->getLibraryPath());

    execSql( QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                     "WHERE Images.id IN (SELECT imageid FROM ImageTags WHERE tagid=%1) "
                     "AND Albums.id=Images.dirid;")
             .arg(tagID), &values );

    for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
    {
        *it = basePath + *it;
    }

    return values;
}

QString AlbumDB::getAlbumURL(int albumID)
{
    QStringList values;
    execSql( QString("SELECT url from Albums where id=%1")
             .arg( albumID), &values);
    return values[0];
}

QDate AlbumDB::getAlbumLowestDate(int albumID)
{
    QStringList values;
    execSql( QString("SELECT MIN(datetime) FROM Images "
                     "WHERE dirid=%1 GROUP BY dirid")
            .arg( albumID ), &values);
    QDate itemDate = QDate::fromString( values[0], Qt::ISODate );
    return itemDate;
}

QDate AlbumDB::getAlbumHighestDate(int albumID)
{
    QStringList values;
    execSql( QString("SELECT MAX(datetime) FROM Images "
                     "WHERE dirid=%1 GROUP BY dirid")
            .arg( albumID ), &values);
    QDate itemDate = QDate::fromString( values[0], Qt::ISODate );
    return itemDate;
}

QDate AlbumDB::getAlbumAverageDate(int albumID)
{
    QStringList values;
    execSql( QString("SELECT datetime FROM Images WHERE dirid=%1")
            .arg( albumID ), &values);

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

void AlbumDB::deleteItem(int albumID, const QString& file)
{
    execSql( QString("DELETE FROM Images "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(albumID)
             .arg(escapeString(file)) );
}

void AlbumDB::setAlbumURL(int albumID, const QString& url)
{
    QString u = escapeString(url);

    // first delete any stale albums left behind
    execSql( QString("DELETE FROM Albums WHERE url = '%1'")
             .arg(u) );

    // now update the album url
    execSql( QString("UPDATE Albums SET url = '%1' WHERE id = %2;")
             .arg(u, QString::number(albumID) ));
}

void AlbumDB::setTagName(int tagID, const QString& name)
{
    execSql( QString("UPDATE Tags SET name='%1' WHERE id=%2;")
             .arg(escapeString(name), QString::number(tagID) ));
}

void AlbumDB::moveItem(int srcAlbumID, const QString& srcName,
                       int dstAlbumID, const QString& dstName)
{

    // first delete any stale database entries if any
    deleteItem(dstAlbumID, dstName);

    execSql( QString("UPDATE Images SET dirid=%1, name='%2' "
                     "WHERE dirid=%3 AND name='%4';")
             .arg(QString::number(dstAlbumID), escapeString(dstName),
                  QString::number(srcAlbumID), escapeString(srcName)) );
}

int AlbumDB::copyItem(int srcAlbumID, const QString& srcName,
                      int dstAlbumID, const QString& dstName)
{
    // check for src == dest
    if (srcAlbumID == dstAlbumID && srcName == dstName)
        return -1;

    // find id of src image
    QStringList values;
    execSql( QString("SELECT id FROM Images "
                     "WHERE dirid=%1 AND name='%2';")
             .arg(QString::number(srcAlbumID), escapeString(srcName)),
             &values);

    if (values.isEmpty())
        return -1;

    int srcId = values[0].toInt();

    // first delete any stale database entries if any
    deleteItem(dstAlbumID, dstName);

    // copy entry in Images table
    execSql( QString("INSERT INTO Images (dirid, name, caption, datetime) "
                     "SELECT %1, '%2', caption, datetime FROM Images "
                     "WHERE id=%3;")
             .arg(QString::number(dstAlbumID), escapeString(dstName),
                  QString::number(srcId)) );

    int dstId = sqlite3_last_insert_rowid(d->dataBase);

    // copy tags
    execSql( QString("INSERT INTO ImageTags (imageid, tagid) "
                     "SELECT %1, tagid FROM ImageTags "
                     "WHERE imageid=%2;")
             .arg(QString::number(dstId), QString::number(srcId)) );

    // copy properties (rating)
    execSql( QString("INSERT INTO ImageProperties (imageid, property, value) "
                     "SELECT %1, property, value FROM ImageProperties "
                     "WHERE imageid=%2;")
             .arg(QString::number(dstId), QString::number(srcId)) );

    return dstId;
}

Q_LLONG AlbumDB::lastInsertedRow()
{
    return sqlite3_last_insert_rowid(d->dataBase);    
}

}  // namespace Digikam

