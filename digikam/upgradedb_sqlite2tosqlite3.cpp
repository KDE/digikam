/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-05
 * Description :
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// Qt includes.

#include <qmap.h>
#include <qpair.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qfileinfo.h>

// KDE includes.

#include <kstandarddirs.h>
#include <kio/global.h>
#include <iostream>

// Local includes.

#include "ddebug.h"
#include "albumdb.h"
#include "albumdb_sqlite2.h"
#include "upgradedb_sqlite2tosqlite3.h"

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

static QString escapeString(QString str)
{
    str.replace( "'", "''" );
    return str;
}

Q_LLONG findOrAddImage(AlbumDB* db, int dirid, const QString& name,
               const QString& caption)
{
    QStringList values;
    
    db->execSql(QString("SELECT id FROM Images WHERE dirid=%1 AND name='%2'")
        .arg(dirid)
        .arg(escapeString(name)), &values);

    if (!values.isEmpty())
    {
    return values.first().toLongLong();
    }
    
    db->execSql(QString("INSERT INTO Images (dirid, name, caption) \n "
            "VALUES(%1, '%2', '%3');")
        .arg(dirid)
        .arg(escapeString(name))
        .arg(escapeString(caption)), &values);

    return db->lastInsertedRow();
}


bool upgradeDB_Sqlite2ToSqlite3(const QString& _libraryPath)
{
    QString libraryPath = QDir::cleanDirPath(_libraryPath);

    QString newDB= libraryPath + "/digikam3.db";

#ifdef NFS_HACK
    newDB = locateLocal("appdata", KIO::encodeFileName(QDir::cleanDirPath(newDB)));
    DDebug() << "NFS: " << newDB << endl;
#endif
    
    AlbumDB db3;
    db3.setDBPath(newDB);
    if (!db3.isValid())
    {
    DWarning() << "Failed to open new Album Database" << endl;
    return false;
    }

    if (db3.getSetting("UpgradedFromSqlite2") == "yes")
    return true;

    QString dbPath = libraryPath + "/digikam.db";

#ifdef NFS_HACK
    dbPath = locateLocal("appdata", KIO::encodeFileName(QDir::cleanDirPath(dbPath)));
    DDebug() << "From NFS: " << dbPath << endl;
#endif
        
    QFileInfo fi(dbPath);

    if (!fi.exists())
    {
    DDebug() << "No old database present. Not upgrading" << endl;
    db3.setSetting("UpgradedFromSqlite2", "yes");
    return true;
    }

    AlbumDB_Sqlite2 db2;
    db2.setDBPath( dbPath );
    if (!db2.isValid())
    {
    DDebug() << "Failed to initialize Old Album Database" << endl;
    return false;
    }

    // delete entries from sqlite3 database
    db3.execSql("DELETE FROM Albums;");
    db3.execSql("DELETE FROM Tags;");
    db3.execSql("DELETE FROM TagsTree;");
    db3.execSql("DELETE FROM Images;");
    db3.execSql("DELETE FROM ImageTags;");
    db3.execSql("DELETE FROM ImageProperties;");
    
    QStringList values;

    // update albums -------------------------------------------------
    
    values.clear();
    db2.execSql("SELECT id, url, date, caption, collection, icon FROM Albums;",
        &values);

    typedef QValueList<_Album> AlbumList;
    AlbumList albumList;

    typedef QMap<QString, int> AlbumMap;
    AlbumMap albumMap;

    db3.beginTransaction();
    for (QStringList::iterator it=values.begin(); it!=values.end();)
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

    db3.execSql(QString("INSERT INTO Albums (id, url, date, caption, collection) "
                "VALUES(%1, '%2', '%3', '%4', '%5');")
            .arg(album.id)
            .arg(escapeString(album.url))
            .arg(escapeString(album.date))
            .arg(escapeString(album.caption))
            .arg(escapeString(album.collection)));
    }
    db3.commitTransaction();

    // update tags -------------------------------------------------
    
    values.clear();
    db2.execSql("SELECT id, pid, name, icon FROM Tags;",
        &values);

    typedef QValueList<_Tag> TagList;
    TagList tagList;

    db3.beginTransaction();
    for (QStringList::iterator it=values.begin(); it!=values.end();)
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

    db3.execSql(QString("INSERT INTO Tags (id, pid, name) "
                "VALUES(%1, %2, '%3');")
            .arg(tag.id)
            .arg(tag.pid)
            .arg(escapeString(tag.name)));
    }
    db3.commitTransaction();

    // update images -------------------------------------------------
    
    values.clear();
    db2.execSql("SELECT dirid, name, caption FROM Images;",
        &values);

    db3.beginTransaction();
    for (QStringList::iterator it=values.begin(); it!=values.end();)
    {
    int dirid   = (*it).toInt();
    ++it;
    QString name    = (*it);
    ++it;
    QString caption = (*it);
    ++it;

    findOrAddImage(&db3, dirid, name, caption);
    }
    db3.commitTransaction();

    // update imagetags -----------------------------------------------
    
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

    Q_LLONG imageid = findOrAddImage(&db3, dirid, name, QString());
    
    db3.execSql(QString("INSERT INTO ImageTags VALUES( %1, %2 )")
            .arg(imageid).arg(tagid));
    }
    db3.commitTransaction();

    // update album icons -------------------------------------------------

    db3.beginTransaction();
    for (AlbumList::iterator it = albumList.begin(); it != albumList.end();
     ++it)
    {
    _Album album = *it;

    if (album.icon.isEmpty())
        continue;

    Q_LLONG imageid = findOrAddImage(&db3, album.id, album.icon, QString());

    db3.execSql(QString("UPDATE Albums SET icon=%1 WHERE id=%2")
            .arg(imageid)
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
        DDebug() << "Could not find album with url: " << url << endl;
        DDebug() << "Most likely an external directory. Rejecting." << endl;
        continue;
    }

    int dirid = it1.data();

    Q_LLONG imageid = findOrAddImage(&db3, dirid, name, QString());;

    db3.execSql(QString("UPDATE Tags SET icon=%1 WHERE id=%2")
            .arg(imageid)
            .arg(tag.id));

    }
    db3.commitTransaction();

    // -- Remove invalid entries ----------------------------------------
    db3.execSql("DELETE FROM Images WHERE dirid=-1");

    // -- update setting entry ------------------------------------------
    db3.setSetting("UpgradedFromSqlite2", "yes");

    DDebug() << "Successfully upgraded database to sqlite3 " << endl;

    // -- Check for db consistency ----------------------------------------

    std::cout << "Checking database consistency" << std::endl;

    
    std::cout << "Checking Albums..................";
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
    db3.execSql(QString("SELECT id FROM Albums WHERE \n"
                "    id=%1 AND \n"
                "    url='%2' AND \n"
                "    date='%3' AND \n"
                "    caption='%4' AND \n"
                "    collection='%5';")
            .arg(album.id)
            .arg(escapeString(album.url))
            .arg(escapeString(album.date))
            .arg(escapeString(album.caption))
            .arg(escapeString(album.collection)), &list, false);
    if (list.size() != 1)
    {
        std::cerr << "Failed" << std::endl;
        DWarning() << "" << endl;
        DWarning() << "Consistency check failed for Album: "
              << album.url << endl;
        return false;
    }
    }
    std::cout << "(" << values.count()/5 << " Albums) "  << "OK" << std::endl;

    
    std::cout << "Checking Tags....................";
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
    db3.execSql(QString("SELECT id FROM Tags WHERE \n"
                "    id=%1 AND \n"
                "    pid=%2 AND \n"
                "    name='%3';")
            .arg(id)
            .arg(pid)
            .arg(escapeString(name)),
            &list, false);
    if (list.size() != 1)
    {
        std::cerr << "Failed" << std::endl;
        DWarning() << "" << endl;
        DWarning() << "Consistency check failed for Tag: "
              << name << endl;
        return false;
    }
    }
    std::cout << "(" << values.count()/3 << " Tags) "  << "OK" << std::endl;

    
    std::cout << "Checking Images..................";
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
    db3.execSql(QString("SELECT Images.id FROM Images, Albums WHERE \n "
                "Albums.url = '%1' AND \n "
                "Images.dirid = Albums.id AND \n "
                "Images.name = '%2' AND \n "
                "Images.caption = '%3';")
            .arg(escapeString(url))
            .arg(escapeString(name))
            .arg(escapeString(caption)),
            &list, false);
    if (list.size() != 1)
    {
        std::cerr << "Failed" << std::endl;
        DWarning() << "" << endl;
        DWarning() << "Consistency check failed for Image: "
              << url << ", " << name << ", " << caption  << endl;
        return false;
    }
    }
    std::cout << "(" << values.count()/3 << " Images) " << "OK" << std::endl;

    
    std::cout << "Checking ImageTags...............";
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
    db3.execSql(QString("SELECT Images.id FROM Albums, Images, ImageTags WHERE \n "
                "Albums.url = '%1' AND \n "
                "Images.dirid = Albums.id AND \n "
                "Images.name = '%2' AND \n "
                "ImageTags.imageid = Images.id AND \n "
                "ImageTags.tagid = %3;")
            .arg(escapeString(url))
            .arg(escapeString(name))
            .arg(tagid),
            &list, false);
    if (list.size() != 1)
    {
        std::cerr << "Failed" << std::endl;
        DWarning() << "" << endl;
        DWarning() << "Consistency check failed for ImageTag: "
              << url << ", " << name << ", " << tagid << endl;
        return false;
    }
    }
    std::cout << "(" << values.count()/3 << " ImageTags) " << "OK" << std::endl;

    std::cout << "Checking Album icons ...............";
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
    db3.execSql(QString("SELECT Images.id FROM Images, Albums WHERE \n "
                "Albums.url = '%1' AND \n "
                "Images.id = Albums.icon AND \n "
                "Images.name = '%2';")
            .arg(escapeString(url))
            .arg(escapeString(icon)), &list);

    if (list.size() != 1)
    {
        std::cerr << "Failed" << std::endl;
        DWarning() << "" << endl;
        DWarning() << "Consistency check failed for Album Icon: "
              << url << ", " << icon << endl;

        return false;
    }
    }
    std::cout << "(" << values.count()/2 << " Album Icons) " << "OK" << std::endl;

    
    std::cout << "Checking Tag icons ...............";
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
        db3.execSql(QString("SELECT id FROM Tags WHERE \n "
                "id = %1 AND \n "
                "iconkde = '%2';")
            .arg(id)
            .arg(escapeString(icon)), &list);

        if (list.size() != 1)
        {
        std::cerr << "Failed" << std::endl;
        DWarning() << "" << endl;
        DWarning() << "Consistency check failed for Tag Icon: "
              << id << ", " << icon << endl;

        return false;
        }
    }
    else
    {
        icon = QDir::cleanDirPath(icon);
        QFileInfo fi(icon.remove(libraryPath));

        QString url  = fi.dirPath(true);
        QString name = fi.fileName();

        QStringList list;

        list.clear();
        db3.execSql(QString("SELECT id FROM Albums WHERE url='%1'")
            .arg(escapeString(url)), &list);
        if (list.isEmpty())
        {
        DWarning() << "Tag icon not in Album Library Path, Rejecting " << endl;
        DWarning() << "(" << icon << ")" << endl;
        continue;
        }

        list.clear();
        db3.execSql(QString("SELECT Images.id FROM Images, Tags WHERE \n "
                " Images.dirid=(SELECT id FROM Albums WHERE url='%1') AND \n "
                " Images.name='%2' AND \n "
                " Tags.id=%3 AND \n "
                " Tags.icon=Images.id")
            .arg(escapeString(url))
            .arg(escapeString(name))
            .arg(id), &list);
        if (list.size() != 1)
        {
        std::cerr << "Failed." << std::endl;
        DWarning() << "" << endl;
        DWarning() << "Consistency check failed for Tag Icon: "
              << id << ", " << icon << endl;

        return false;
        }
        
    }
    }
    std::cout << "(" << values.count()/2 << " Tag Icons) " << "OK" << std::endl;

    std::cout << "" << std::endl;
    std::cout << "All Tests: A-OK" << std::endl;
    
    return true;
}

}  // namespace Digikam
