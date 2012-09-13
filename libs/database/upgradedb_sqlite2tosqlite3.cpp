/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-05
 * Description : SQlite 2 to SQlite 3 interface.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#include "upgradedb_sqlite2tosqlite3.h"

// Qt includes

#include <QMap>
#include <QPair>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QList>

// KDE includes

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kio/global.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "albumdb_sqlite2.h"

namespace Digikam
{

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

qlonglong findOrAddImage(DatabaseBackend* backend, int dirid, const QString& name,
                         const QString& caption)
{
    QList<QVariant> values;

    backend->execSql(QString("SELECT id FROM Images WHERE dirid=? AND name=?"),
                     dirid, name, &values);

    if (!values.isEmpty())
    {
        return values.first().toLongLong();
    }

    QVariant id;
    backend->execSql(QString("INSERT INTO Images (dirid, name, caption) \n "
                             "VALUES(?, ?, ?);"),
                     dirid, name, caption, 0, &id);

    return id.toInt();
}


bool upgradeDB_Sqlite2ToSqlite3(AlbumDB* albumDB, DatabaseBackend* backend, const QString& sql2DBPath)
{
    QString libraryPath = QDir::cleanPath(sql2DBPath);

    /*
    QString newDB= libraryPath + "/digikam3.db";

    #ifdef NFS_HACK
    newDB = locateLocal("appdata", KIO::encodeFileName(QDir::cleanPath(newDB)));
    kDebug() << "NFS: " << newDB;
    #endif

    AlbumDB db3;
    albumDB->setDBPath(newDB);
    if (!albumDB->isValid())
    {
        kWarning() << "Failed to open new Album Database";
        return false;
    }
    */

    //DatabaseAccess access;

    if (albumDB->getSetting("UpgradedFromSqlite2") == "yes")
    {
        return true;
    }

    QString dbPath = libraryPath + "/digikam3.db";

    /*
    #ifdef NFS_HACK
    dbPath = locateLocal("appdata", KIO::encodeFileName(QDir::cleanPath(dbPath)));
    kDebug() << "From NFS: " << dbPath;
    #endif
    */

    QFileInfo fi(dbPath);

    if (!fi.exists())
    {
        kDebug() << "No old database present. Not upgrading";
        albumDB->setSetting("UpgradedFromSqlite2", "yes");
        return true;
    }

    AlbumDB_Sqlite2 db2;
    db2.setDBPath( dbPath );

    if (!db2.isValid())
    {
        kDebug() << "Failed to initialize Old Album Database";
        return false;
    }

    // delete entries from sqlite3 database
    backend->execSql("DELETE FROM Albums;");
    backend->execSql("DELETE FROM Tags;");
    backend->execSql("DELETE FROM TagsTree;");
    backend->execSql("DELETE FROM Images;");
    backend->execSql("DELETE FROM ImageTags;");
    backend->execSql("DELETE FROM ImageProperties;");

    QStringList values;

    // update albums -------------------------------------------------

    values.clear();
    db2.execSql("SELECT id, url, date, caption, collection, icon FROM Albums;",
                &values);

    typedef QList<_Album> AlbumList;
    AlbumList albumList;

    typedef QMap<QString, int> AlbumMap;
    AlbumMap albumMap;

    backend->beginTransaction();

    for (QStringList::const_iterator it=values.constBegin(); it!=values.constEnd();)
    {
        _Album album;

        album.id     = (*it).toInt();
        ++it;
        album.url    = (*it);
        ++it;
        album.date   = (*it);
        ++it;
        album.caption    = (*it);
        ++it;
        album.collection = (*it);
        ++it;
        album.icon   = (*it);
        ++it;

        albumList.append(album);
        albumMap.insert(album.url, album.id);

        QList<QVariant> boundValues;
        boundValues << album.id << album.url << album.date << album.caption << album.collection;

        backend->execSql(QString("INSERT INTO Albums (id, url, date, caption, collection) "
                                 "VALUES(?, ?, ?, ?, ?);"),
                         boundValues);
    }

    backend->commitTransaction();

    // update tags -------------------------------------------------

    values.clear();
    db2.execSql("SELECT id, pid, name, icon FROM Tags;",
                &values);

    typedef QList<_Tag> TagList;
    TagList tagList;

    backend->beginTransaction();

    for (QStringList::const_iterator it=values.constBegin(); it!=values.constEnd();)
    {
        _Tag tag;

        tag.id   = (*it).toInt();
        ++it;
        tag.pid  = (*it).toInt();
        ++it;
        tag.name = (*it);
        ++it;
        tag.icon = (*it);
        ++it;

        tagList.append(tag);

        backend->execSql(QString("INSERT INTO Tags (id, pid, name) "
                                 "VALUES(?, ?, ?);"),
                         tag.id, tag.pid, tag.name);
    }

    backend->commitTransaction();

    // update images -------------------------------------------------

    values.clear();
    db2.execSql("SELECT dirid, name, caption FROM Images;",
                &values);

    backend->beginTransaction();

    for (QStringList::const_iterator it=values.constBegin(); it!=values.constEnd();)
    {
        int dirid   = (*it).toInt();
        ++it;
        QString name    = (*it);
        ++it;
        QString caption = (*it);
        ++it;

        findOrAddImage(backend, dirid, name, caption);
    }

    backend->commitTransaction();

    // update imagetags -----------------------------------------------

    values.clear();
    db2.execSql("SELECT dirid, name, tagid FROM ImageTags;",
                &values);
    backend->beginTransaction();

    for (QStringList::const_iterator it=values.constBegin(); it!=values.constEnd();)
    {
        int dirid = (*it).toInt();
        ++it;

        QString name = (*it);
        ++it;

        int tagid = (*it).toInt();
        ++it;

        qlonglong imageid = findOrAddImage(backend, dirid, name, QString());

        backend->execSql(QString("INSERT INTO ImageTags VALUES(?, ?)"),
                         imageid, tagid);
    }

    backend->commitTransaction();

    // update album icons -------------------------------------------------

    backend->beginTransaction();

    for (AlbumList::const_iterator it = albumList.constBegin(); it != albumList.constEnd();
         ++it)
    {
        _Album album = *it;

        if (album.icon.isEmpty())
        {
            continue;
        }

        qlonglong imageid = findOrAddImage(backend, album.id, album.icon, QString());

        backend->execSql(QString("UPDATE Albums SET icon=? WHERE id=?"),
                         imageid, album.id);
    }

    backend->commitTransaction();

    // -- update tag icons ---------------------------------------------------

    backend->beginTransaction();

    for (TagList::const_iterator it = tagList.constBegin(); it != tagList.constEnd(); ++it)
    {
        _Tag tag = *it;

        if (tag.icon.isEmpty())
        {
            continue;
        }

        QFileInfo fi(tag.icon);

        if (fi.isRelative())
        {
            backend->execSql(QString("UPDATE Tags SET iconkde=? WHERE id=?"),
                             tag.icon, tag.id);
            continue;
        }

        tag.icon = QDir::cleanPath(tag.icon);
        fi.setFile(tag.icon.remove(libraryPath));

        QString url  = fi.absolutePath();
        QString name = fi.fileName();

        AlbumMap::const_iterator it1 = albumMap.constFind(url);

        if (it1 == albumMap.constEnd())
        {
            kDebug() << "Could not find album with url: " << url;
            kDebug() << "Most likely an external directory. Rejecting.";
            continue;
        }

        int dirid = it1.value();

        qlonglong imageid = findOrAddImage(backend, dirid, name, QString());;

        backend->execSql(QString("UPDATE Tags SET icon=? WHERE id=?"),
                         imageid, tag.id);

    }

    backend->commitTransaction();

    // -- Remove invalid entries ----------------------------------------
    backend->execSql("DELETE FROM Images WHERE dirid=-1");

    // -- update setting entry ------------------------------------------
    albumDB->setSetting("UpgradedFromSqlite2", "yes");

    kDebug() << "Successfully upgraded database to sqlite3 ";

    // -- Check for db consistency ----------------------------------------

    /*
    kDebug() << "Checking database consistency";


    kDebug() << "Checking Albums..................";
    values.clear();
    db2.execSql("SELECT id, url, date, caption, collection FROM Albums;", &values);
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        _Album album;
        album.id     = (*it).toInt();
        ++it;
        album.url    = (*it);
        ++it;
        album.date   = (*it);
        ++it;
        album.caption    = (*it);
        ++it;
        album.collection = (*it);
        ++it;

        QStringList list;
        backend->execSql(QString("SELECT id FROM Albums WHERE \n"
                "    id=%1 AND \n"
                "    url='%2' AND \n"
                "    date='%3' AND \n"
                "    caption='%4' AND \n"
                "    collection='%5';")
                .arg(album.id)
                .arg(backend->escapeString(album.url))
                .arg(backend->escapeString(album.date))
                .arg(backend->escapeString(album.caption))
                .arg(backend->escapeString(album.collection)), &list, false);
        if (list.size() != 1)
        {
            kError() << "Failed";
            kWarning() << "";
            kWarning() << "Consistency check failed for Album: "
                       << album.url;
            return false;
        }
    }
    kDebug() << " (" << values.count()/5 << " Albums) "  << "OK";


    kDebug() << "Checking Tags....................";
    values.clear();
    db2.execSql("SELECT id, pid, name FROM Tags;", &values);
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        int id       = (*it).toInt();
        ++it;
        int pid      = (*it).toInt();
        ++it;
        QString name = (*it);
        ++it;

        QStringList list;
        backend->execSql(QString("SELECT id FROM Tags WHERE \n"
                "    id=%1 AND \n"
                "    pid=%2 AND \n"
                "    name='%3';")
                .arg(id)
                .arg(pid)
                .arg(backend->escapeString(name)),
        &list, false);
        if (list.size() != 1)
        {
            kError() << "Failed";
            kWarning() << "";
            kWarning() << "Consistency check failed for Tag: "
                       << name;
            return false;
        }
    }
    kDebug() << " (" << values.count()/3 << " Tags) "  << "OK";


    kDebug() << "Checking Images..................";
    values.clear();
    db2.execSql("SELECT Albums.url, Images.name, Images.caption "
            "FROM Images, Albums WHERE Albums.id=Images.dirid;", &values);
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        QString url  = (*it);
        ++it;
        QString name = (*it);
        ++it;
        QString caption = (*it);
        ++it;

        QStringList list;
        backend->execSql(QString("SELECT Images.id FROM Images, Albums WHERE \n "
                "Albums.url = '%1' AND \n "
                "Images.dirid = Albums.id AND \n "
                "Images.name = '%2' AND \n "
                "Images.caption = '%3';")
                .arg(backend->escapeString(url))
                .arg(backend->escapeString(name))
                .arg(backend->escapeString(caption)),
        &list, false);
        if (list.size() != 1)
        {
            kError() << "Failed";
            kWarning() << "";
            kWarning() << "Consistency check failed for Image: "
                       << url << ", " << name << ", " << caption;
            return false;
        }
    }
    kDebug() << " (" << values.count()/3 << " Images) " << "OK";


    kDebug() << "Checking ImageTags...............";
    values.clear();
    db2.execSql("SELECT Albums.url, ImageTags.name, ImageTags.tagid "
            "FROM ImageTags, Albums WHERE \n "
            "   Albums.id=ImageTags.dirid;", &values);
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        QString url   = (*it);
        ++it;
        QString name  = (*it);
        ++it;
        int tagid = (*it).toInt();
        ++it;

        QStringList list;
        backend->execSql(QString("SELECT Images.id FROM Albums, Images, ImageTags WHERE \n "
                "Albums.url = '%1' AND \n "
                "Images.dirid = Albums.id AND \n "
                "Images.name = '%2' AND \n "
                "ImageTags.imageid = Images.id AND \n "
                "ImageTags.tagid = %3;")
                .arg(backend->escapeString(url))
                .arg(backend->escapeString(name))
                .arg(tagid),
        &list, false);
        if (list.size() != 1)
        {
            kError() << "Failed";
            kWarning() << "";
            kWarning() << "Consistency check failed for ImageTag: "
                       << url << ", " << name << ", " << tagid;
            return false;
        }
    }
    kDebug() << " (" << values.count()/3 << " ImageTags) " << "OK";

    kDebug() << "Checking Album icons ...............";
    values.clear();
    db2.execSql("SELECT url, icon FROM Albums;", &values);
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        QString url    = (*it);
        ++it;
        QString icon   = (*it);
        ++it;

        if (icon.isEmpty())
            continue;

        QStringList list;
        backend->execSql(QString("SELECT Images.id FROM Images, Albums WHERE \n "
                "Albums.url = '%1' AND \n "
                "Images.id = Albums.icon AND \n "
                "Images.name = '%2';")
                .arg(backend->escapeString(url))
                .arg(backend->escapeString(icon)), &list);

        if (list.size() != 1)
        {
            kError() << "Failed";
            kWarning() << "";
            kWarning() << "Consistency check failed for Album Icon: "
                       << url << ", " << icon;

            return false;
        }
    }
    kDebug() << " (" << values.count()/2 << " Album Icons) " << "OK";


    kDebug() << "Checking Tag icons ...............";
    values.clear();
    db2.execSql("SELECT id, icon FROM Tags;", &values);
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        int id       = (*it).toInt();
        ++it;
        QString icon = (*it);
        ++it;

        if (icon.isEmpty())
            continue;

        if (!icon.startsWith("/"))
        {
            QStringList list;
            backend->execSql(QString("SELECT id FROM Tags WHERE \n "
                    "id = %1 AND \n "
                    "iconkde = '%2';")
                    .arg(id)
                    .arg(backend->escapeString(icon)), &list);

            if (list.size() != 1)
            {
                kError() << "Failed";
                kWarning() << "";
                kWarning() << "Consistency check failed for Tag Icon: "
                           << id << ", " << icon;

                return false;
            }
        }
        else
        {
            icon = QDir::cleanPath(icon);
            QFileInfo fi(icon.remove(libraryPath));

            QString url  = fi.absolutePath();
            QString name = fi.fileName();

            QStringList list;

            list.clear();
            backend->execSql(QString("SELECT id FROM Albums WHERE url='%1'")
                    .arg(backend->escapeString(url)), &list);
            if (list.isEmpty())
            {
                kWarning() << "Tag icon not in Album Library Path, Rejecting ";
                kWarning() << "(" << icon << ")";
                continue;
            }

            list.clear();
            backend->execSql(QString("SELECT Images.id FROM Images, Tags WHERE \n "
                    " Images.dirid=(SELECT id FROM Albums WHERE url='%1') AND \n "
                    " Images.name='%2' AND \n "
                    " Tags.id=%3 AND \n "
                    " Tags.icon=Images.id")
                    .arg(backend->escapeString(url))
                    .arg(backend->escapeString(name))
                    .arg(id), &list);
            if (list.size() != 1)
            {
                kError() << "Failed.";
                kWarning() << "";
                kWarning() << "Consistency check failed for Tag Icon: "
                           << id << ", " << icon;

                return false;
            }

        }
    }
    kDebug() << " (" << values.count()/2 << " Tag Icons) " << "OK";

    kDebug() << "";
    kDebug() << "All Tests: A-OK";
    */

    return true;
}

}  // namespace Digikam
