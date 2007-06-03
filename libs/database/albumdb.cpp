/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description :database album interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include "databaseattributeswatch.h"
#include "databasebackend.h"
#include "albumdb.h"

namespace Digikam
{

class AlbumDBPriv
{

public:

    AlbumDBPriv()
    {
        sql = 0;
    }

    DatabaseBackend *sql;
    IntList  recentlyAssignedTags;
};

AlbumDB::AlbumDB(DatabaseBackend *backend)
{
    d = new AlbumDBPriv;
    d->sql = backend;
}

AlbumDB::~AlbumDB()
{
    delete d;
}

AlbumInfo::List AlbumDB::scanAlbums()
{
    AlbumInfo::List aList;

    QString basePath(DatabaseAccess::albumRoot());

    QStringList values;
    execSql( "SELECT A.id, A.url, A.date, A.caption, A.collection, B.url, I.name \n "
             "FROM Albums AS A \n "
             "  LEFT OUTER JOIN Images AS I ON A.icon=I.id \n"
             "  LEFT OUTER JOIN Albums AS B ON B.id=I.dirid;", &values);

    QString iconAlbumUrl, iconName;

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        AlbumInfo info;

        info.albumRoot = basePath;
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

    QString basePath(DatabaseAccess::albumRoot());

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

QValueList<AlbumShortInfo> AlbumDB::getAlbumShortInfos()
{
    QStringList values;
    execSql( QString("SELECT id, url from Albums;"),
             &values);

    QValueList<AlbumShortInfo> albumList;

    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        AlbumShortInfo info;

        info.id        = (*it).toInt();
        ++it;
        info.url       = (*it);
        ++it;
        info.albumRoot = DatabaseAccess::albumRoot();

        albumList << info;
    }

    return albumList;
}

QStringList AlbumDB::getSubalbumsForPath(const QString &albumRoot,
                                         const QString& path,
                                         bool onlyDirectSubalbums)
{
    QString subURL = path;
    if (!path.endsWith("/"))
        subURL += '/';
    subURL = escapeString(subURL);

    QStringList values;

    if (onlyDirectSubalbums)
    {
        execSql( QString("SELECT id, url FROM Albums WHERE url LIKE '") +
                 subURL + QString("%' ") + QString("AND url NOT LIKE '") +
                 subURL + QString("%/%'; "),
                 &values );
    }
    else
    {
        execSql( QString("SELECT id, url FROM Albums WHERE url LIKE '") +
                 subURL + QString("%'; "),
                 &values );
    }

    return values;
}

int AlbumDB::addAlbum(const QString &albumRoot, const QString& url,
                      const QString& caption,
                      const QDate& date, const QString& collection)
{
    execSql( QString("REPLACE INTO Albums (url, date, caption, collection) "
                     "VALUES('%1', '%2', '%3', '%4');")
             .arg(escapeString(url),
                  date.toString(Qt::ISODate),
                  escapeString(caption),
                  escapeString(collection)));

    int id = d->sql->lastInsertedRow();
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

    QString basePath(DatabaseAccess::albumRoot());
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
    if (!execSql( QString("INSERT INTO Tags (pid, name) "
                          "VALUES( %1, '%2')")
                  .arg(parentTagID)
                  .arg(escapeString(name))))
    {
        return -1;
    }

    int id = d->sql->lastInsertedRow();

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
    
    QString basePath(DatabaseAccess::albumRoot());

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
    QString str("INSERT INTO Searches (name, url) \n"
                "VALUES('$$@@$$', '$$##$$');");
    str.replace("$$@@$$", escapeString(name));
    str.replace("$$##$$", escapeString(url.url()));

    if (!execSql(str))
    {
        return -1;
    }

    return d->sql->lastInsertedRow();
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
        return QString();
    else
        return values[0];
}

bool AlbumDB::execSql(const QString& sql, QStringList* const values,
                      QString *errMsg, bool debug)
{
    return d->sql->execSql(sql, values, errMsg, debug);
}

QString AlbumDB::escapeString(QString str) const
{
    return d->sql->escapeString(str);
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
        return QString();
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
        return QString();
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

ItemShortInfo AlbumDB::getItemShortInfo(Q_LLONG imageID)
{
    QStringList values;

    execSql( QString("SELECT Images.name, Albums.url, Albums.id "
                     "FROM Images "
                     "LEFT OUTER JOIN Albums "
                     "ON Albums.id=Images.dirid "
                     "WHERE Images.id=%1;")
             .arg(imageID),
             &values );

    ItemShortInfo info;

    if (!values.isEmpty())
    {
        info.id        = imageID;
        info.itemName  = values[0];
        info.albumRoot = DatabaseAccess::albumRoot();
        info.album     = values[1];
        info.albumID   = values[2].toInt();
    }

    return info;
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

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageComment);
}

void AlbumDB::setItemCaption(int albumID, const QString& name, const QString& caption)
{
    /*
    QStringList values;

    execSql( QString("UPDATE Images SET caption='%1' "
                     "WHERE dirid=%2 AND name='%3';")
             .arg(escapeString(caption),
                  QString::number(albumID),
                  escapeString(name)) );
    */

    // easier because of attributes watch
    return setItemCaption(getImageId(albumID, name), caption);
}

void AlbumDB::addItemTag(Q_LLONG imageID, int tagID)
{
    execSql( QString("REPLACE INTO ImageTags (imageid, tagid) "
                     "VALUES(%1, %2);")
                 .arg(imageID)
                 .arg(tagID) );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageTags);

    if (!d->recentlyAssignedTags.contains(tagID))
    {
        d->recentlyAssignedTags.push_front(tagID);
        if (d->recentlyAssignedTags.size() > 10)
            d->recentlyAssignedTags.pop_back();
    }
}

void AlbumDB::addItemTag(int albumID, const QString& name, int tagID)
{
    /*
    execSql( QString("REPLACE INTO ImageTags (imageid, tagid) \n "
                     "(SELECT id, %1 FROM Images \n "
                     " WHERE dirid=%2 AND name='%3');")
             .arg(tagID)
             .arg(albumID)
             .arg(escapeString(name)) );
    */

    // easier because of attributes watch
    return addItemTag(getImageId(albumID, name), tagID);
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

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageTags);
}

void AlbumDB::removeItemAllTags(Q_LLONG imageID)
{
    execSql( QString("DELETE FROM ImageTags "
                     "WHERE imageID=%1;")
             .arg(imageID) );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageTags);
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

    QString libraryPath = DatabaseAccess::albumRoot() + '/';
    for (QStringList::iterator it = values.begin(); it != values.end();
         ++it)
    {
        *it = libraryPath + *it;
    }

    return values;
}

QValueList<QPair<QString, QDateTime> > AlbumDB::getItemsAndDate()
{
    QStringList values;
    execSql( "SELECT name, datetime FROM Images;", &values );

    QValueList<QPair<QString, QDateTime> > data;
    for ( QStringList::iterator it = values.begin(); it != values.end(); )
    {
        QPair<QString, QDateTime> pair;
        pair.first  = *it;
        ++it;
        pair.second = QDateTime::fromString( *it,  Qt::ISODate );
        ++it;

        if (!pair.second.isValid())
            continue;

        data << pair;
    }
    return data;
}


int AlbumDB::getAlbumForPath(const QString &albumRoot, const QString& folder, bool create)
{
    QStringList values;
    execSql( QString("SELECT id FROM Albums WHERE url ='%1';")
            .arg( escapeString(folder) ), &values);

    int albumID = -1;
    if (values.isEmpty() && create)
    {
        albumID = addAlbum(albumRoot, folder, QString(), QDate::currentDate(), QString());
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

    Q_LLONG item = d->sql->lastInsertedRow();

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
                tagID = addTag(parentTagID, (*tagName), QString(), 0);

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

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageDate);

    return true;
}

bool AlbumDB::setItemDate(int albumID, const QString& name,
                          const QDateTime& datetime)
{
    /*
    execSql ( QString ("UPDATE Images SET datetime='%1'"
                       "WHERE dirid=%2 AND name='%3';")
              .arg(datetime.toString(Qt::ISODate),
                   QString::number(albumID),
                   escapeString(name)) );

    return true;
    */
    // easier because of attributes watch
    return setItemDate(getImageId(albumID, name), datetime);
}

void AlbumDB::setItemRating(Q_LLONG imageID, int rating)
{
    execSql ( QString ("REPLACE INTO ImageProperties "
                       "(imageid, property, value) "
                       "VALUES(%1, '%2', '%3');")
              .arg(imageID)
              .arg("Rating")
              .arg(rating) );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageRating);
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

QStringList AlbumDB::getItemURLsInAlbum(int albumID, ItemSortOrder sortOrder)
{
    QStringList values;

    QString basePath(DatabaseAccess::albumRoot());

    QString sqlString;
    switch(sortOrder)
    {
        case ByItemName:
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                                 "WHERE Albums.id=%1 AND Albums.id=Images.dirid "
                                 "ORDER BY Images.name COLLATE NOCASE;")
                        .arg(albumID);
            break;
        case ByItemPath:
            // Dont collate on the path - this is to maintain the same behaviour
            // that happens when sort order is "By Path"
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                                 "WHERE Albums.id=%1 AND Albums.id=Images.dirid "
                                 "ORDER BY Albums.url,Images.name;")
                        .arg(albumID);
            break;
        case ByItemDate:
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                                 "WHERE Albums.id=%1 AND Albums.id=Images.dirid "
                                 "ORDER BY Images.datetime;")
                        .arg(albumID);
            break;
        case ByItemRating:
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums, ImageProperties "
                                 "WHERE Albums.id=%1 AND Albums.id=Images.dirid "
                                 "AND Images.id = ImageProperties.imageid "
                                 "AND ImageProperties.property='Rating' "
                                 "ORDER BY ImageProperties.value DESC;")
                        .arg(albumID);
            break;
        case NoItemSorting:
        default:
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                                 "WHERE Albums.id=%1 AND Albums.id=Images.dirid;")
                        .arg(albumID);
            break;
    }
    execSql(sqlString, &values);

    for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
    {
        *it = basePath + *it;
    }

    return values;
}

LLongList AlbumDB::getItemIDsInAlbum(int albumID)
{
    LLongList itemIDs;
    QStringList values;

    execSql( QString("SELECT id FROM Images WHERE dirid=%1;")
                .arg(albumID), &values );

    for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
    {
        itemIDs << (*it).toLong();
    }

    return itemIDs;
}

QStringList AlbumDB::getItemURLsInTag(int tagID, bool recursive)
{
    QStringList values;

    QString basePath(DatabaseAccess::albumRoot());

    QString imagesIdClause;
    if (recursive)
        imagesIdClause = QString("SELECT imageid FROM ImageTags "
                                 " WHERE tagid=%1 "
                                 " OR tagid IN (SELECT id FROM TagsTree WHERE pid=%2)")
                                .arg(tagID).arg(tagID);
    else
        imagesIdClause = QString("SELECT imageid FROM ImageTags WHERE tagid=%1").arg(tagID);

    execSql( QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                     "WHERE Images.id IN (%1) "
                     "AND Albums.id=Images.dirid;")
             .arg(imagesIdClause), &values );

    for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
    {
        *it = basePath + *it;
    }

    return values;
}

LLongList AlbumDB::getItemIDsInTag(int tagID, bool recursive)
{
    LLongList itemIDs;
    QStringList values;

    if (recursive)
        execSql( QString("SELECT imageid FROM ImageTags "
                         " WHERE tagid=%1 "
                         " OR tagid IN (SELECT id FROM TagsTree WHERE pid=%2);")
                .arg(tagID).arg(tagID), &values );
    else
        execSql( QString("SELECT imageid FROM ImageTags WHERE tagid=%1;")
                .arg(tagID), &values );

    for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
    {
        itemIDs << (*it).toLong();
    }

    return itemIDs;
}

QString AlbumDB::getAlbumURL(int albumID)
{
    QStringList values;
    execSql( QString("SELECT url from Albums WHERE id=%1")
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

void AlbumDB::renameAlbum(int albumID, const QString& newUrl, bool renameSubalbums)
{
    QString oldUrl = getAlbumURL(albumID);

    // first delete any stale albums left behind
    execSql( QString("DELETE FROM Albums WHERE url = '%1'")
             .arg(escapeString(newUrl)) );

    // now update the album url
    execSql( QString("UPDATE Albums SET url = '%1' WHERE id = %2;")
             .arg(escapeString(newUrl), QString::number(albumID) ));

    if (renameSubalbums)
    {
        // now find the list of all subalbums which need to be updated
        QStringList values;
        execSql( QString("SELECT url FROM Albums WHERE url LIKE '%1/%';")
                 .arg(escapeString(oldUrl)), &values );

        // and update their url
        QString newChildURL;
        for (QStringList::iterator it = values.begin(); it != values.end(); ++it)
        {
            newChildURL = *it;
            newChildURL.replace(oldUrl, newUrl);
            execSql(QString("UPDATE Albums SET url='%1' WHERE url='%2'")
                    .arg(escapeString(newChildURL),
                         escapeString(*it)));
        }
    }
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

    int dstId = d->sql->lastInsertedRow();

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

bool AlbumDB::copyAlbumProperties(int srcAlbumID, int dstAlbumID)
{
    if (srcAlbumID == dstAlbumID)
        return true;

    QStringList values;
    execSql( QString("SELECT date, caption, collection, icon "
                     "FROM Albums WHERE id=%1;").arg(srcAlbumID),
             &values );

    if (values.isEmpty())
    {
        DWarning() << k_funcinfo << " src album ID " << srcAlbumID << " does not exist" << endl;
        return false;
    }

    execSql( QString("UPDATE Albums SET date='%1', caption='%2', "
                     "collection='%3', icon=%4 ")
                         .arg(values[0],
                              values[1],
                              values[2],
                              values[3]) +
                         QString( " WHERE id=%1" )
                         .arg(dstAlbumID) );
    return true;
}

}  // namespace Digikam

