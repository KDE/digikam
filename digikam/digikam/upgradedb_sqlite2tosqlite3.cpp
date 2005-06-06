/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-05
 * Description :
 *
 * Copyright 2005 by Renchi Raju

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
#include <qmap.h>
#include <qpair.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qfileinfo.h>

#include "albumdb.h"
#include "albumdb_sqlite2.h"

#include "upgradedb_sqlite2tosqlite3.h"

struct _Album
{
    int     id;
    QString url;
    QString date;
    QString caption;
    QString collection;
    QString icon;
};


struct _Tag
{
    int     id;
    int     pid;
    QString name;
    QString icon;
};

struct _Image
{
    int     id;
    int     dirid;
    QString name;
    QString datetime;
    QString caption;
};

static QString escapeString(QString str)
{
    str.replace( "'", "''" );
    return str;
}

bool upgradeDB_Sqlite2ToSqlite3(const QString& libraryPath)
{
    AlbumDB db3;
    db3.setDBPath(libraryPath + "/digikam3.db");

    if (!db3.isValid())
    {
        kdWarning() << "Failed to open new Album Database" << endl;
        return false;
    }

    if (db3.getSetting("UpgradedFromSqlite2") == "yes")
        return true;

    kdDebug() << "Upgrading database to sqlite3 " << endl;

    QFileInfo fi(libraryPath + "/digikam.db");
    if (!fi.exists())
    {
        // no old database.
        return true;
    }

    AlbumDB_Sqlite2 db2;
    db2.setDBPath(libraryPath + "/digikam.db");
    if (!db2.isValid())
    {
        kdWarning() << "Failed to initialize Old Album Database" << endl;
        return false;
    }

    QStringList values;

    values.clear();
    db2.execSql("SELECT id, url, date, caption, collection, icon FROM Albums;",
                &values);

    // 1. get list of albums from sqlite2.
    // 2. insert list of albums into sqlite3.
    // 3. get list of tags from sqlite3.
    // 4. insert list of tags into sqlite3.
    // 5. get list of images from sqlite2.
    // 6. insert list of images into sqlite3 with new album id. (get new id of images)
    // 7. get list of imagetags from sqlite2.
    // 8. update list of imagetags with id of images
    // 9. insert list of imagetags into sqlite3
    // 10. do something with the icons


    typedef QValueList<_Album> AlbumList;
    AlbumList albumList;

    typedef QMap<QString, int> AlbumMap;
    AlbumMap albumMap;

    db3.beginTransaction();
    for (QStringList::iterator it=values.begin(); it!=values.end();)
    {
        _Album album;

        album.id         = (*it).toInt();
        ++it;
        album.url        = (*it);
        ++it;
        album.date       = (*it);
        ++it;
        album.caption    = (*it);
        ++it;
        album.collection = (*it);
        ++it;
        album.icon   = (*it);
        ++it;

        albumList.append(album);
        albumMap.insert(album.url, album.id);

        db3.execSql(QString("INSERT INTO Albums (id, url, date, caption, collection) "
                            "VALUES(%1, '%2', '%3', '%4', '%5');")
                    .arg(album.id)
                    .arg(escapeString(album.url))
                    .arg(album.date)
                    .arg(escapeString(album.caption))
                    .arg(escapeString(album.collection)));
    }
    db3.commitTransaction();

    values.clear();
    db2.execSql("SELECT id, pid, name, icon FROM Tags;",
                &values);


    typedef QValueList<_Tag> TagList;
    TagList tagList;

    db3.beginTransaction();
    for (QStringList::iterator it=values.begin(); it!=values.end();)
    {
        _Tag tag;

        tag.id         = (*it).toInt();
        ++it;
        tag.pid        = (*it).toInt();
        ++it;
        tag.name       = (*it);
        ++it;
        tag.icon   = (*it);
        ++it;

        tagList.append(tag);

        db3.execSql(QString("INSERT INTO Tags (id, pid, name) "
                            "VALUES(%1, %2, '%3');")
                    .arg(tag.id)
                    .arg(tag.pid)
                    .arg(tag.name));
    }
    db3.commitTransaction();

    typedef QPair<int,QString> OldImageIDType;
    typedef QMap<OldImageIDType, int> OldNewImageMapType;

    OldNewImageMapType oldNewImageMap;

    values.clear();
    db2.execSql("SELECT dirid, name, datetime, caption FROM Images;",
                &values);

    db3.beginTransaction();
    for (QStringList::iterator it=values.begin(); it!=values.end();)
    {
        _Image image;

        image.dirid    = (*it).toInt();
        ++it;
        image.name     = (*it);
        ++it;
        image.datetime = (*it);
        ++it;
        image.caption  = (*it);
        ++it;

        image.id = db3.addItem(image.dirid, image.name,
                               QDateTime(), image.caption);

        oldNewImageMap.insert(OldImageIDType(image.dirid,image.name),
                              image.id);
    }
    db3.commitTransaction();

    values.clear();
    db2.execSql("SELECT dirid, name, tagid FROM ImageTags;",
                &values);
    db3.beginTransaction();
    for (QStringList::iterator it=values.begin(); it!=values.end();)
    {
        int dirid = (*it).toInt();
        ++it;

        QString name = (*it);
        ++it;

        int tagid = (*it).toInt();
        ++it;

        OldNewImageMapType::iterator it =
            oldNewImageMap.find(OldImageIDType(dirid, name));

        if (it == oldNewImageMap.end())
        {
            continue;
        }

        int id = it.data();

        db3.execSql(QString("INSERT INTO ImageTags VALUES( %1, %2 )")
                    .arg(id).arg(tagid));
    }
    db3.commitTransaction();

    // update album icons
    db3.beginTransaction();
    for (AlbumList::iterator it = albumList.begin(); it != albumList.end(); ++it)
    {
        _Album album = *it;

        if (album.icon.isEmpty())
            continue;

        OldNewImageMapType::iterator it =
            oldNewImageMap.find(OldImageIDType(album.id, album.icon));

        if (it == oldNewImageMap.end())
        {
            continue;
        }

        db3.execSql(QString("UPDATE Albums SET icon=%1 WHERE id=%2")
                    .arg(it.data())
                    .arg(album.id));
    }
    db3.commitTransaction();

    // -- update album icons ---------------------------------------------------

    db3.beginTransaction();
    for (AlbumList::iterator it = albumList.begin(); it != albumList.end(); ++it)
    {
        _Album album = *it;

        if (album.icon.isEmpty())
            continue;

        OldNewImageMapType::iterator it =
            oldNewImageMap.find(OldImageIDType(album.id, album.icon));

        if (it == oldNewImageMap.end())
        {
            continue;
        }

        db3.execSql(QString("UPDATE Albums SET icon=%1 WHERE id=%2")
                    .arg(it.data())
                    .arg(album.id));
    }
    db3.commitTransaction();

    // -- update tag icons ---------------------------------------------------

    db3.beginTransaction();
    for (TagList::iterator it = tagList.begin(); it != tagList.end(); ++it)
    {
        _Tag tag = *it;

        if (tag.icon.isEmpty())
            continue;

        QFileInfo fi(tag.icon);
        if (fi.isRelative())
        {
            db3.execSql(QString("UPDATE Tags SET iconkde='%1' WHERE id=%2")
                        .arg(escapeString(tag.icon))
                        .arg(tag.id));
            continue;
        }

        tag.icon = QDir::cleanDirPath(tag.icon);
        fi.setFile(tag.icon.remove(libraryPath));

        QString url  = fi.dirPath(true);
        QString name = fi.fileName();

        AlbumMap::iterator it1 = albumMap.find(url);
        if (it1 == albumMap.end())
        {
            continue;
        }

        int dirid = it1.data();

        OldNewImageMapType::iterator it2 = oldNewImageMap.find(OldImageIDType(dirid, name));
        if (it2 == oldNewImageMap.end())
        {
            continue;
        }

        int imageid = it2.data();

        db3.execSql(QString("UPDATE Tags SET icon=%1 WHERE id=%2")
                    .arg(imageid)
                    .arg(tag.id));

    }
    db3.commitTransaction();

    db3.setSetting("UpgradedFromSqlite2", "yes");

    kdDebug() << "Successfully upgraded database to sqlite3 " << endl;

    return true;
}
