/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description :database album interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "albumdb.h"

// C ANSI includes

extern "C"
{
#include <sys/time.h>
}

// C++ includes

#include <cstdio>
#include <cstdlib>
#include <ctime>

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QDir>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "databasebackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"

namespace Digikam
{

class AlbumDBPriv
{

public:

    AlbumDBPriv()
    {
        db = 0;
    }

    DatabaseBackend *db;
    QList<int>  recentlyAssignedTags;
};

AlbumDB::AlbumDB(DatabaseBackend *backend)
       : d(new AlbumDBPriv)
{
    d->db = backend;
}

AlbumDB::~AlbumDB()
{
    delete d;
}

QList<AlbumRootInfo> AlbumDB::getAlbumRoots()
{
    QList<AlbumRootInfo> list;

    QList<QVariant> values;
    d->db->execSql( "SELECT id, label, status, type, identifier, specificPath FROM AlbumRoots;", &values );

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        AlbumRootInfo info;
        info.id           = (*it).toInt();
        ++it;
        info.label        = (*it).toString();
        ++it;
        info.status       = (*it).toInt();
        ++it;
        info.type         = (AlbumRoot::Type)(*it).toInt();
        ++it;
        info.identifier   = (*it).toString();
        ++it;
        info.specificPath = (*it).toString();
        ++it;

        list << info;
    }

    return list;
}

int AlbumDB::addAlbumRoot(AlbumRoot::Type type, const QString& identifier, const QString& specificPath, const QString& label)
{
    QVariant id;
    d->db->execSql( QString("REPLACE INTO AlbumRoots (type, label, status, identifier, specificPath) "
                            "VALUES(?, ?, 0, ?, ?);"),
                    (int)type, label, identifier, specificPath, 0, &id);

    d->db->recordChangeset(AlbumRootChangeset(id.toInt(), AlbumRootChangeset::Added));
    return id.toInt();
}

void AlbumDB::deleteAlbumRoot(int rootId)
{
    d->db->execSql( QString("DELETE FROM AlbumRoots WHERE id=?;"),
                    rootId );
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::Deleted));
}

void AlbumDB::setAlbumRootLabel(int rootId, const QString& newLabel)
{
    d->db->execSql( QString("UPDATE AlbumRoots SET label=? WHERE id=?;"),
                    newLabel, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

void AlbumDB::changeAlbumRootType(int rootId, AlbumRoot::Type newType)
{
    d->db->execSql( QString("UPDATE AlbumRoots SET type=? WHERE id=?;"),
                    (int)newType, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}



AlbumInfo::List AlbumDB::scanAlbums()
{
    AlbumInfo::List aList;

    QList<QVariant> values;
    d->db->execSql( "SELECT A.albumRoot, A.id, A.relativePath, A.date, A.caption, A.collection, B.albumRoot, B.relativePath, I.name \n "
                    "FROM Albums AS A \n "
                    "  LEFT JOIN Images AS I ON A.icon=I.id \n"
                    "  LEFT JOIN Albums AS B ON B.id=I.album \n"
                    " WHERE A.albumRoot != 0;", // exclude stale albums
                    &values);

    QString iconAlbumUrl, iconName;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        AlbumInfo info;

        info.albumRootId = (*it).toInt();
        ++it;
        info.id = (*it).toInt();
        ++it;
        info.relativePath = (*it).toString();
        ++it;
        info.date = QDate::fromString((*it).toString(), Qt::ISODate);
        ++it;
        info.caption = (*it).toString();
        ++it;
        info.category = (*it).toString();
        ++it;
        info.iconAlbumRootId = (*it).toInt(); // will be 0 if null
        ++it;
        iconAlbumUrl = (*it).toString();
        ++it;
        iconName = (*it).toString();
        ++it;

        if (!iconName.isEmpty())
            info.iconRelativePath = iconAlbumUrl + '/' + iconName;

        aList.append(info);
    }

    return aList;
}

TagInfo::List AlbumDB::scanTags()
{
    TagInfo::List tList;

    QList<QVariant> values;
    d->db->execSql( "SELECT T.id, T.pid, T.name, A.relativePath, I.name, T.iconkde, A.albumRoot \n "
                    "FROM Tags AS T \n"
                    "  LEFT JOIN Images AS I ON I.id=T.icon \n "
                    "  LEFT JOIN Albums AS A ON A.id=I.album; ", &values );

    QString iconName, iconKDE, albumURL;
    int iconAlbumRootId;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        TagInfo info;

        info.id     = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = (*it).toString();
        ++it;
        albumURL    = (*it).toString();
        ++it;
        iconName    = (*it).toString();
        ++it;
        iconKDE     = (*it).toString();
        ++it;
        iconAlbumRootId = (*it).toInt(); // will be 0 if null
        ++it;

        if (albumURL.isEmpty())
        {
            info.icon = iconKDE;
        }
        else
        {
            info.iconAlbumRootId  = iconAlbumRootId;
            info.iconRelativePath = albumURL + '/' + iconName;
        }

        tList.append(info);
    }

    return tList;
}

SearchInfo::List AlbumDB::scanSearches()
{
    SearchInfo::List searchList;

    QList<QVariant> values;
    d->db->execSql( "SELECT id, type, name, query FROM Searches;", &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        SearchInfo info;

        info.id    = (*it).toInt();
        ++it;
        info.type  = (DatabaseSearch::Type)(*it).toInt();
        ++it;
        info.name  = (*it).toString();
        ++it;
        info.query = (*it).toString();
        ++it;

        searchList.append(info);
    }

    return searchList;
}

QList<AlbumShortInfo> AlbumDB::getAlbumShortInfos()
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT Albums.id, Albums.relativePath, Albums.albumRoot from Albums; "),
                    &values);

    QList<AlbumShortInfo> albumList;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        AlbumShortInfo info;

        info.id           = (*it).toLongLong();
        ++it;
        info.relativePath = (*it).toString();
        ++it;
        info.albumRootId  = (*it).toInt();
        ++it;

        albumList << info;
    }

    return albumList;
}

/*
QStringList AlbumDB::getSubalbumsForPath(const QString& albumRoot,
                                         const QString& path,
                                         bool onlyDirectSubalbums)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);
    if (location.isNull())
        return QStringList();

    QString subURL = path;
    if (!path.endsWith("/"))
        subURL += '/';
    subURL = (subURL);

    QList<QVariant> values;

    if (onlyDirectSubalbums)
    {
        d->db->execSql( QString("SELECT relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE '") +
                        subURL + QString("%' ") + QString("AND relativePath NOT LIKE '") +
                        subURL + QString("%/%'; "),
                        location.id(),
                        &values );
    }
    else
    {
        d->db->execSql( QString("SELECT relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE '") +
                        subURL + QString("%'; "),
                        location.id(),
                        &values );
    }

    QStringList subalbums;
    QString albumRootPath = location.albumRootPath();
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        subalbums << albumRootPath + it->toString();
    return subalbums;
}
*/

/*
int AlbumDB::addAlbum(const QString& albumRoot, const QString& relativePath,
                      const QString& caption,
                      const QDate& date, const QString& collection)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);
    if (location.isNull())
        return -1;

    return addAlbum(location.id(), relativePath, caption, date, collection);
}
*/

int AlbumDB::addAlbum(int albumRootId, const QString& relativePath,
                      const QString& caption,
                      const QDate& date, const QString& collection)
{
    QVariant id;
    QList<QVariant> boundValues;
    boundValues << albumRootId << relativePath << date.toString(Qt::ISODate) << caption << collection;

    d->db->execSql( QString("REPLACE INTO Albums (albumRoot, relativePath, date, caption, collection) "
                            "VALUES(?, ?, ?, ?, ?);"),
                    boundValues, 0, &id);

    d->db->recordChangeset(AlbumChangeset(id.toInt(), AlbumChangeset::Added));
    return id.toInt();
}

void AlbumDB::setAlbumCaption(int albumID, const QString& caption)
{
    d->db->execSql( QString("UPDATE Albums SET caption=? WHERE id=?;"),
                    caption, albumID );
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void AlbumDB::setAlbumCategory(int albumID, const QString& category)
{
    // TODO : change "collection" propertie in DB ALbum table to "category"
    d->db->execSql( QString("UPDATE Albums SET collection=? WHERE id=?;"),
                    category, albumID );
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void AlbumDB::setAlbumDate(int albumID, const QDate& date)
{
    d->db->execSql( QString("UPDATE Albums SET date=? WHERE id=?;"),
                    date.toString(Qt::ISODate),
                    albumID );
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void AlbumDB::setAlbumIcon(int albumID, qlonglong iconID)
{
    d->db->execSql( QString("UPDATE Albums SET icon=? WHERE id=?;"),
                    iconID,
                    albumID );
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

bool AlbumDB::getAlbumIcon(int albumID, int *albumRootId, QString *iconRelativePath)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT B.relativePath, I.name, B.albumRoot \n "
                            "FROM Albums AS A \n "
                            "  LEFT JOIN Images AS I ON I.id=A.icon \n "
                            "  LEFT JOIN Albums AS B ON B.id=I.album \n "
                            "WHERE A.id=?;"),
                    albumID, &values );
    if (values.isEmpty())
        return false;

    QList<QVariant>::const_iterator it = values.constBegin();
    QString album     = (*it).toString();
    ++it;
    QString iconName  = (*it).toString();
    ++it;
    *albumRootId = (*it).toInt();

    *iconRelativePath = album + '/' + iconName;

    return !iconName.isEmpty();
}

void AlbumDB::deleteAlbum(int albumID)
{
    d->db->execSql( QString("DELETE FROM Albums WHERE id=?;"),
                    albumID );
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Deleted));
}

void AlbumDB::makeStaleAlbum(int albumID)
{
    // We need to work around the table constraint, no we want to delete older stale albums with
    // the same relativePath, and adjust relativePaths depending on albumRoot.
    QList<QVariant> values;

    // retrieve information
    d->db->execSql( QString("SELECT Albums.albumRoot, Albums.relativePath from Albums WHERE id=?;"),
                    albumID, &values);

    if (values.isEmpty())
        return;

    // prepend albumRootId to relativePath. relativePath is unused and officially undefined after this call.
    QString newRelativePath = values[0].toString() + '-' + values[1].toString();

    // delete older stale albums
    d->db->execSql( QString("DELETE FROM Albums WHERE albumRoot=0 AND relativePath=?;"),
                    newRelativePath );

    // now do our update
    d->db->execSql( QString("UPDATE Albums SET albumRoot=0, relativePath=? WHERE id=?;"),
                    newRelativePath, albumID );

    // for now, we make no distinction to deleteAlbums wrt to changeset
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Deleted));
}

void AlbumDB::deleteStaleAlbums()
{
    d->db->execSql( QString("DELETE FROM Albums WHERE albumRoot=0;") );
    // deliberately no changeset here, is done above
}

int AlbumDB::addTag(int parentTagID, const QString& name, const QString& iconKDE,
                    qlonglong iconID)
{
    QVariant id;
    if (!d->db->execSql( QString("INSERT INTO Tags (pid, name) "
                                 "VALUES( ?, ?);"),
                         parentTagID,
                         name,
                         0, &id) )
    {
        return -1;
    }

    if (!iconKDE.isEmpty())
    {
        d->db->execSql( QString("UPDATE Tags SET iconkde=? WHERE id=?;"),
                        iconKDE,
                        id.toInt() );
    }
    else
    {
        d->db->execSql( QString("UPDATE Tags SET icon=? WHERE id=?;"),
                        iconID,
                        id.toInt());
    }

    d->db->recordChangeset(TagChangeset(id.toInt(), TagChangeset::Added));
    return id.toInt();
}

void AlbumDB::deleteTag(int tagID)
{
    d->db->execSql( QString("DELETE FROM Tags WHERE id=?;"),
                    tagID );
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Deleted));
}

void AlbumDB::setTagIcon(int tagID, const QString& iconKDE, qlonglong iconID)
{
    if (!iconKDE.isEmpty())
    {
        d->db->execSql( QString("UPDATE Tags SET iconkde=?, icon=0 WHERE id=?;"),
                        iconKDE, tagID );
    }
    else
    {
        d->db->execSql( QString("UPDATE Tags SET icon=? WHERE id=?;"),
                        iconID, tagID );
    }
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::IconChanged));
}

bool AlbumDB::getTagIcon(int tagID, int *iconAlbumRootId, QString *iconAlbumRelativePath, QString *icon)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT A.relativePath, I.name, T.iconkde, A.albumRoot \n "
                            "FROM Tags AS T \n "
                            "  LEFT JOIN Images AS I ON I.id=T.icon \n "
                            "  LEFT JOIN Albums AS A ON A.id=I.album \n "
                            "WHERE T.id=?;"),
                    tagID, &values );

    if (values.isEmpty())
        return false;

    QString iconName, iconKDE, albumURL;

    QList<QVariant>::const_iterator it = values.constBegin();

    albumURL    = (*it).toString();
    ++it;
    iconName    = (*it).toString();
    ++it;
    iconKDE     = (*it).toString();
    ++it;

    *iconAlbumRootId = (*it).toInt();
    ++it;

    if (albumURL.isEmpty())
    {
        *iconAlbumRelativePath = QString();
        *icon = iconKDE;
        return !iconKDE.isEmpty();
    }
    else
    {
        *iconAlbumRelativePath = albumURL + '/' + iconName;
        *icon = QString();
        return true;
    }
}

void AlbumDB::setTagParentID(int tagID, int newParentTagID)
{
    d->db->execSql( QString("UPDATE Tags SET pid=? WHERE id=?;"),
                    newParentTagID, tagID );
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Reparented));
}

int AlbumDB::addSearch(DatabaseSearch::Type type, const QString& name, const QString& query)
{
    QVariant id;
    if (!d->db->execSql(QString("INSERT INTO Searches (type, name, query) VALUES(?, ?, ?);"),
                        type, name, query, 0, &id) )
    {
        return -1;
    }

    d->db->recordChangeset(SearchChangeset(id.toInt(), SearchChangeset::Added));
    return id.toInt();
}

void AlbumDB::updateSearch(int searchID, DatabaseSearch::Type type,
                           const QString& name, const QString &query)
{
    d->db->execSql(QString("UPDATE Searches SET type=?, name=?, query=? WHERE id=?"),
                   type, name, query, searchID);
    d->db->recordChangeset(SearchChangeset(searchID, SearchChangeset::Changed));
}

void AlbumDB::deleteSearch(int searchID)
{
    d->db->execSql( QString("DELETE FROM Searches WHERE id=?"),
                    searchID );
    d->db->recordChangeset(SearchChangeset(searchID, SearchChangeset::Deleted));
}

void AlbumDB::deleteSearches(DatabaseSearch::Type type)
{
    d->db->execSql( QString("DELETE FROM Searches WHERE type=?"),
                    type );
    d->db->recordChangeset(SearchChangeset(0, SearchChangeset::Deleted));
}

QString AlbumDB::getSearchQuery(int searchId)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT query FROM Searches WHERE id=?;"),
                    searchId, &values );

    if (values.isEmpty())
        return QString();
    else
        return values.first().toString();
}

SearchInfo AlbumDB::getSearchInfo(int searchId)
{
    SearchInfo info;

    QList<QVariant> values;
    d->db->execSql( "SELECT id, type, name, query FROM Searches WHERE id=?;",
                    searchId, &values);

    if (values.size() == 4)
    {
        QList<QVariant>::const_iterator it = values.constBegin();
        info.id    = (*it).toInt();
        ++it;
        info.type  = (DatabaseSearch::Type)(*it).toInt();
        ++it;
        info.name  = (*it).toString();
        ++it;
        info.query = (*it).toString();
        ++it;
    }

    return info;
}

void AlbumDB::setSetting(const QString& keyword,
                         const QString& value )
{
    d->db->execSql( QString("REPLACE into Settings VALUES (?,?);"),
                    keyword, value );
}

QString AlbumDB::getSetting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT value FROM Settings "
                            "WHERE keyword=?;"),
                    keyword, &values );

    if (values.isEmpty())
        return QString();
    else
        return values.first().toString();
}

// helper method
static QStringList joinMainAndUserFilterString(const QString &filter, const QString &userFilter)
{
    QSet<QString> filterSet;
    QStringList userFilterList;

    filterSet = filter.split(';', QString::SkipEmptyParts).toSet();
    userFilterList = userFilter.split(';', QString::SkipEmptyParts);
    foreach (const QString& userFormat, userFilterList)
    {
        if (userFormat.startsWith('-'))
            filterSet.remove(userFormat.mid(1));
        else
            filterSet << userFormat;
    }
    return filterSet.toList();
}

void AlbumDB::getFilterSettings(QStringList *imageFilter, QStringList *videoFilter, QStringList *audioFilter)
{
    QString imageFormats, videoFormats, audioFormats, userImageFormats, userVideoFormats, userAudioFormats;

    if (imageFilter)
    {
        imageFormats = getSetting("databaseImageFormats");
        userImageFormats = getSetting("databaseUserImageFormats");
        *imageFilter = joinMainAndUserFilterString(imageFormats, userImageFormats);
    }

    if (videoFilter)
    {
        videoFormats = getSetting("databaseVideoFormats");
        userVideoFormats = getSetting("databaseUserVideoFormats");
        *videoFilter = joinMainAndUserFilterString(videoFormats, userVideoFormats);
    }

    if (audioFilter)
    {
        audioFormats = getSetting("databaseAudioFormats");
        userAudioFormats = getSetting("databaseUserAudioFormats");
        *audioFilter = joinMainAndUserFilterString(audioFormats, userAudioFormats);
    }
}

void AlbumDB::getUserFilterSettings(QString *imageFilterString, QString *videoFilterString, QString *audioFilterString)
{
    if (imageFilterString)
        *imageFilterString = getSetting("databaseUserImageFormats");
    if (videoFilterString)
        *videoFilterString = getSetting("databaseUserVideoFormats");
    if (audioFilterString)
        *audioFilterString = getSetting("databaseUserAudioFormats");
}

void AlbumDB::setFilterSettings(const QStringList& imageFilter, const QStringList& videoFilter, const QStringList& audioFilter)
{
    setSetting("databaseImageFormats", imageFilter.join(";"));
    setSetting("databaseVideoFormats", videoFilter.join(";"));
    setSetting("databaseAudioFormats", audioFilter.join(";"));
}

// helper method
static QStringList cleanUserFilterString(const QString &filterString)
{
    // splits by either ; or space, removes "*.", trims
    QStringList filterList;

    QString wildcard("*.");
    QString minusWildcard("-*.");
    QChar dot('.');
    QString minusDot("-.");
    QChar sep( ';' );
    int i = filterString.indexOf( sep );
    if ( i == -1 && filterString.indexOf( ' ') != -1 )
        sep = QChar( ' ' );

    QStringList sepList = filterString.split(sep, QString::SkipEmptyParts);
    foreach (const QString& f, sepList)
    {
        if (f.startsWith(wildcard))
            filterList << f.mid(2).trimmed().toLower();
        else if (f.startsWith(minusWildcard))
            filterList << '-' + f.mid(3).trimmed().toLower();
        else if (f.startsWith(dot))
            filterList << f.mid(1).trimmed().toLower();
        else if (f.startsWith(minusDot))
            filterList << '-' + f.mid(2).trimmed().toLower();
        else
            filterList << f.trimmed().toLower();
    }
    return filterList;
}

void AlbumDB::setUserFilterSettings(const QString& imageFilterString, const QString& videoFilterString, const QString& audioFilterString)
{
    setUserFilterSettings(cleanUserFilterString(imageFilterString),
                          cleanUserFilterString(videoFilterString),
                          cleanUserFilterString(audioFilterString));
}

void AlbumDB::setUserFilterSettings(const QStringList& imageFilter, const QStringList& videoFilter, const QStringList& audioFilter)
{
    setSetting("databaseUserImageFormats", imageFilter.join(";"));
    setSetting("databaseUserVideoFormats", videoFilter.join(";"));
    setSetting("databaseUserAudioFormats", audioFilter.join(";"));
}

void AlbumDB::addToUserImageFilterSettings(const QString& filterString)
{
    QStringList addList = cleanUserFilterString(filterString);

    QStringList currentList = getSetting("databaseUserImageFormats").split(';', QString::SkipEmptyParts);

    // merge lists
    foreach(const QString& addedFilter, addList)
    {
        if (!currentList.contains(addedFilter))
            currentList << addedFilter;
    }

    setSetting("databaseUserImageFormats", currentList.join(";"));
}

QUuid AlbumDB::databaseUuid()
{
    QString uuidString = getSetting("databaseUUID");
    QUuid uuid = QUuid(uuidString);
    if (uuidString.isNull() || uuid.isNull())
    {
        uuid = QUuid::createUuid();
        setSetting("databaseUUID", uuid.toString());
    }
    return uuid;
}

/*
QString AlbumDB::getItemCaption(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT caption FROM Images "
                            "WHERE id=?;"),
                    imageID, &values );

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

QString AlbumDB::getItemCaption(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT caption FROM Images "
                            "WHERE dirid=? AND name=?;"),
                    albumID,
                    name,
                    &values );

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}


QDateTime AlbumDB::getItemDate(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT datetime FROM Images "
                            "WHERE id=?;"),
                    imageID,
                    &values );

    if (!values.isEmpty())
        return QDateTime::fromString(values.first().toString(), Qt::ISODate);
    else
        return QDateTime();
}

QDateTime AlbumDB::getItemDate(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT datetime FROM Images "
                            "WHERE dirid=? AND name=?;"),
                    albumID,
                    name,
                    &values );

    if (values.isEmpty())
        return QDateTime::fromString(values.first().toString(), Qt::ISODate);
    else
        return QDateTime();
}
*/

qlonglong AlbumDB::getImageId(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT id FROM Images "
                            "WHERE album=? AND name=?;"),
                    albumID,
                    name,
                    &values );

    if (values.isEmpty())
        return -1;
    else
        return values.first().toLongLong();
}

QStringList AlbumDB::getItemTagNames(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT name FROM Tags \n "
                            "WHERE id IN (SELECT tagid FROM ImageTags \n "
                            "             WHERE imageid=?) \n "
                            "ORDER BY name;"),
                    imageID,
                    &values );

    QStringList names;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
        names << it->toString();
    return names;
}

QList<int> AlbumDB::getItemTagIDs(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT tagid FROM ImageTags \n "
                            "WHERE imageID=?;"),
                    imageID,
                    &values );

    QList<int> ids;

    if (values.isEmpty())
        return ids;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
        ids << it->toInt();
    return ids;
}

ItemShortInfo AlbumDB::getItemShortInfo(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT Images.name, Albums.albumRoot, Albums.relativePath, Albums.id "
                            "FROM Images "
                            "  LEFT JOIN Albums ON Albums.id=Images.album "
                            "WHERE Images.id=?;"),
                    imageID,
                    &values );

    ItemShortInfo info;

    if (!values.isEmpty())
    {
        info.id          = imageID;
        info.itemName    = values[0].toString();
        info.albumRootID = values[1].toInt();
        info.album       = values[2].toString();
        info.albumID     = values[3].toInt();
    }

    return info;
}

bool AlbumDB::hasTags(const QList<qlonglong>& imageIDList)
{
    QList<int> ids;

    if (imageIDList.isEmpty())
        return false;

    QList<QVariant> values;
    QList<QVariant> boundValues;

    QString sql = QString("SELECT count(tagid) FROM ImageTags "
                          "WHERE imageid=? ");
    boundValues << imageIDList.first();

    QList<qlonglong>::const_iterator it = imageIDList.constBegin();
    ++it;
    for (; it != imageIDList.constEnd(); ++it)
    {
        sql += QString(" OR imageid=? ");
        boundValues << (*it);
    }

    sql += QString(";");
    d->db->execSql( sql, boundValues, &values );

    if (values.isEmpty() || values.first().toInt() == 0)
        return false;
    else
        return true;
}

QList<int> AlbumDB::getItemCommonTagIDs(const QList<qlonglong>& imageIDList)
{
    QList<int> ids;

    if (imageIDList.isEmpty())
        return ids;

    QList<QVariant> values;
    QList<QVariant> boundValues;

    QString sql = QString("SELECT DISTINCT tagid FROM ImageTags "
                          "WHERE imageid=? ");
    boundValues << imageIDList.first();

    QList<qlonglong>::const_iterator it = imageIDList.constBegin();
    ++it;
    for (; it != imageIDList.constEnd(); ++it)
    {
        sql += QString(" OR imageid=? ");
        boundValues << (*it);
    }

    sql += QString(";");
    d->db->execSql( sql, boundValues, &values );

    if (values.isEmpty())
        return ids;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
        ids << it->toInt();
    return ids;
}



QVariantList AlbumDB::getImagesFields(qlonglong imageID, DatabaseFields::Images fields)
{
    QVariantList values;
    if (fields != DatabaseFields::ImagesNone)
    {
        QString query("SELECT ");
        QStringList fieldNames = imagesFieldList(fields);
        query += fieldNames.join(", ");
        query += (" FROM Images WHERE id=?;");

        d->db->execSql(query, imageID, &values);

        // Convert date times to QDateTime, they come as QString
        if (fields & DatabaseFields::ModificationDate && !values.isEmpty())
        {
            int index = fieldNames.indexOf("modificationDate");
            values[index] = (values[index].isNull() ? QDateTime()
                              : QDateTime::fromString(values[index].toString(), Qt::ISODate));
        }
    }
    return values;
}

QVariantList AlbumDB::getImageInformation(qlonglong imageID, DatabaseFields::ImageInformation fields)
{
    QVariantList values;
    if (fields != DatabaseFields::ImageInformationNone)
    {
        QString query("SELECT ");
        QStringList fieldNames = imageInformationFieldList(fields);
        query += fieldNames.join(", ");
        query += (" FROM ImageInformation WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // Convert date times to QDateTime, they come as QString
        if (fields & DatabaseFields::CreationDate && !values.isEmpty())
        {
            int index = fieldNames.indexOf("creationDate");
            values[index] = (values[index].isNull() ? QDateTime()
                              : QDateTime::fromString(values[index].toString(), Qt::ISODate));
        }
        if (fields & DatabaseFields::DigitizationDate && !values.isEmpty())
        {
            int index = fieldNames.indexOf("digitizationDate");
            values[index] = (values[index].isNull() ? QDateTime()
                              : QDateTime::fromString(values[index].toString(), Qt::ISODate));
        }
    }
    return values;
}

QVariantList AlbumDB::getImageMetadata(qlonglong imageID, DatabaseFields::ImageMetadata fields)
{
    QVariantList values;
    if (fields != DatabaseFields::ImageMetadataNone)
    {
        QString query("SELECT ");
        QStringList fieldNames = imageMetadataFieldList(fields);
        query += fieldNames.join(", ");
        query += (" FROM ImageMetadata WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // For some reason REAL values may come as QString QVariants. Convert here.
        if (values.size() == fieldNames.size() &&
            (fields & DatabaseFields::Aperture ||
             fields & DatabaseFields::FocalLength ||
             fields & DatabaseFields::FocalLength35 ||
             fields & DatabaseFields::ExposureTime ||
             fields & DatabaseFields::SubjectDistance)
           )
        {
            for (int i=0; i<values.size(); ++i)
            {
                if (values[i].type() == QVariant::String &&
                    (fieldNames[i] == "aperture" ||
                     fieldNames[i] == "focalLength" ||
                     fieldNames[i] == "focalLength35" ||
                     fieldNames[i] == "exposureTime" ||
                     fieldNames[i] == "subjectDistance")
                   )
                    values[i] = values[i].toDouble();
            }
        }
    }
    return values;
}

QVariantList AlbumDB::getImagePosition(qlonglong imageID, DatabaseFields::ImagePositions fields)
{
    QVariantList values;
    if (fields != DatabaseFields::ImagePositionsNone)
    {
        QString query("SELECT ");
        QStringList fieldNames = imagePositionsFieldList(fields);
        query += fieldNames.join(", ");
        query += (" FROM ImagePositions WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // For some reason REAL values may come as QString QVariants. Convert here.
        if (values.size() == fieldNames.size() &&
            (fields & DatabaseFields::LatitudeNumber ||
             fields & DatabaseFields::LongitudeNumber ||
             fields & DatabaseFields::Altitude ||
             fields & DatabaseFields::PositionOrientation ||
             fields & DatabaseFields::PositionTilt ||
             fields & DatabaseFields::PositionRoll ||
             fields & DatabaseFields::PositionAccuracy)
           )
        {
            for (int i=0; i<values.size(); ++i)
            {
                if (values[i].type() == QVariant::String &&
                    (fieldNames[i] == "latitudeNumber" ||
                     fieldNames[i] == "longitudeNumber" ||
                     fieldNames[i] == "altitude" ||
                     fieldNames[i] == "orientation" ||
                     fieldNames[i] == "tilt" ||
                     fieldNames[i] == "roll" ||
                     fieldNames[i] == "accuracy")
                   )
                    values[i] = values[i].toDouble();
            }
        }
    }
    return values;
}

void AlbumDB::addImageInformation(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageInformation fields)
{
    if (fields == DatabaseFields::ImageInformationNone)
        return;

    QString query("REPLACE INTO ImageInformation ( imageid, ");

    QStringList fieldNames = imageInformationFieldList(fields);
    Q_ASSERT(fieldNames.size()==infos.size());
    query += fieldNames.join(", ");

    query += " ) VALUES (";
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += ");";

    QVariantList boundValues;
    boundValues << imageID;
    // Take care for datetime values
    if (fields & DatabaseFields::CreationDate || fields & DatabaseFields::DigitizationDate)
    {
        foreach (const QVariant& value, infos)
        {
            if (value.type() == QVariant::DateTime || value.type() == QVariant::Date)
                boundValues << value.toDateTime().toString(Qt::ISODate);
            else
                boundValues << value;
        }
    }
    else
        boundValues << infos;

    d->db->execSql( query, boundValues );
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::changeImageInformation(qlonglong imageId, const QVariantList& infos,
                                     DatabaseFields::ImageInformation fields)
{
    if (fields == DatabaseFields::ImageInformationNone)
        return;

    QString query("UPDATE ImageInformation SET ");

    QStringList fieldNames = imageInformationFieldList(fields);
    Q_ASSERT(fieldNames.size()==infos.size());
    query += fieldNames.join("=?,");

    query += "=? WHERE imageid=?;";

    QVariantList boundValues;
    // Take care for datetime values
    if (fields & DatabaseFields::CreationDate || fields & DatabaseFields::DigitizationDate)
    {
        foreach (const QVariant& value, infos)
        {
            if (value.type() == QVariant::DateTime || value.type() == QVariant::Date)
                boundValues << value.toDateTime().toString(Qt::ISODate);
            else
                boundValues << value;
        }
        boundValues << imageId;
    }
    else
        boundValues << infos << imageId;

    d->db->execSql( query, boundValues );
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void AlbumDB::addImageMetadata(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageMetadata fields)
{
    if (fields == DatabaseFields::ImageMetadataNone)
        return;

    QString query("REPLACE INTO ImageMetadata ( imageid, ");

    QStringList fieldNames = imageMetadataFieldList(fields);
    Q_ASSERT(fieldNames.size()==infos.size());
    query += fieldNames.join(", ");

    query += " ) VALUES (";
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += ");";

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql( query, boundValues );
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::changeImageMetadata(qlonglong imageId, const QVariantList& infos,
                                  DatabaseFields::ImageMetadata fields)
{
    if (fields == DatabaseFields::ImageMetadataNone)
        return;

    QString query("UPDATE ImageMetadata SET ");

    QStringList fieldNames = imageMetadataFieldList(fields);
    Q_ASSERT(fieldNames.size()==infos.size());
    query += fieldNames.join("=?,");

    query += "=? WHERE imageid=?;";

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql( query, boundValues );
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void AlbumDB::addImagePosition(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImagePositions fields)
{
    if (fields == DatabaseFields::ImagePositionsNone)
        return;

    QString query("REPLACE INTO ImagePositions ( imageid, ");

    QStringList fieldNames = imagePositionsFieldList(fields);
    Q_ASSERT(fieldNames.size()==infos.size());
    query += fieldNames.join(", ");

    query += " ) VALUES (";
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += ");";

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql( query, boundValues );
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::changeImagePosition(qlonglong imageId, const QVariantList& infos,
                                   DatabaseFields::ImagePositions fields)
{
    if (fields == DatabaseFields::ImagePositionsNone)
        return;

    QString query("UPDATE ImagePositions SET ");

    QStringList fieldNames = imagePositionsFieldList(fields);
    Q_ASSERT(fieldNames.size()==infos.size());
    query += fieldNames.join("=?,");

    query += "=? WHERE imageid=?;";

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql( query, boundValues );
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void AlbumDB::removeImagePosition(qlonglong imageid)
{
    d->db->execSql( QString("DELETE FROM ImagePositions WHERE imageid=?;"),
                    imageid );

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::ImagePositionsAll));
}

QList<CommentInfo> AlbumDB::getImageComments(qlonglong imageID)
{
    QList<CommentInfo> list;

    QList<QVariant> values;
    d->db->execSql( QString("SELECT id, type, language, author, date, comment "
                            "FROM ImageComments WHERE imageid=?;"),
                    imageID, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        CommentInfo info;
        info.imageId  = imageID;

        info.id       = (*it).toInt();
        ++it;
        info.type     = (DatabaseComment::Type)(*it).toInt();
        ++it;
        info.language = (*it).toString();
        ++it;
        info.author   = (*it).toString();
        ++it;
        info.date     = ((*it).isNull() ? QDateTime() : QDateTime::fromString((*it).toString(), Qt::ISODate));
        ++it;
        info.comment  = (*it).toString();
        ++it;

        list << info;
    }

    return list;
}

int AlbumDB::setImageComment(qlonglong imageID, const QString& comment, DatabaseComment::Type type,
                             const QString& language, const QString& author, const QDateTime& date)
{
    QVariantList boundValues;
    boundValues << imageID << (int)type << language << author << date << comment;

    QVariant id;
    d->db->execSql( QString("REPLACE INTO ImageComments "
                            "( imageid, type, language, author, date, comment ) "
                            " VALUES (?,?,?,?,?,?);"),
                    boundValues, 0, &id);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::ImageCommentsAll));
    return id.toInt();
}

void AlbumDB::changeImageComment(int commentId, qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageComments fields)
{
    if (fields == DatabaseFields::ImageCommentsNone)
        return;

    QString query("UPDATE ImageComments SET ");

    QStringList fieldNames = imageCommentsFieldList(fields);
    Q_ASSERT(fieldNames.size()==infos.size());
    query += fieldNames.join("=?,");
    query += "=? WHERE id=?;";

    QVariantList boundValues;
    boundValues << infos << commentId;

    d->db->execSql( query, boundValues );
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::removeImageComment(int commentid, qlonglong imageid)
{
    d->db->execSql( QString("DELETE FROM ImageComments WHERE id=?;"),
                    commentid );

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::ImageCommentsAll));
}

QString AlbumDB::getImageProperty(qlonglong imageID, const QString& property)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT value FROM ImageProperties "
                            "WHERE imageid=? and property=?;"),
                    imageID, property,
                    &values);

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

void AlbumDB::setImageProperty(qlonglong imageID, const QString& property, const QString& value)
{
    d->db->execSql( QString ("REPLACE INTO ImageProperties "
                             "(imageid, property, value) "
                             "VALUES(?, ?, ?);"),
                    imageID, property, value);
}

QList<CopyrightInfo> AlbumDB::getImageCopyright(qlonglong imageID, const QString& property)
{
    QList<CopyrightInfo> list;

    QList<QVariant> values;
    if (property.isNull())
    {
        d->db->execSql( QString("SELECT property, value, extraValue FROM ImageCopyright "
                                "WHERE imageid=?;"),
                        imageID, &values);
    }
    else
    {
        d->db->execSql( QString("SELECT property, value, extraValue FROM ImageCopyright "
                                "WHERE imageid=? and property=?;"),
                        imageID, property, &values);
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        CopyrightInfo info;
        info.id = imageID;

        info.property   = (*it).toString();
        ++it;
        info.value      = (*it).toString();
        ++it;
        info.extraValue = (*it).toString();
        ++it;

        list << info;
    }

    return list;
}

void AlbumDB::setImageCopyrightProperty(qlonglong imageID, const QString& property,
                                        const QString& value, const QString& extraValue,
                                        CopyrightPropertyUnique uniqueness)
{
    if (uniqueness == PropertyUnique)
    {
        d->db->execSql( QString("DELETE FROM ImageCopyright "
                                "WHERE imageid=? AND property=?;"),
                        imageID, property );
    }
    else if (uniqueness == PropertyExtraValueUnique)
    {
        d->db->execSql( QString("DELETE FROM ImageCopyright "
                                "WHERE imageid=? AND property=? AND extraValue=?;"),
                        imageID, property, extraValue );
    }

    d->db->execSql( QString("REPLACE INTO ImageCopyright "
                            "(imageid, property, value, extraValue) "
                            "VALUES(?, ?, ?, ?);"),
                    imageID, property, value, extraValue);
}

bool AlbumDB::hasHaarFingerprints()
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT imageid FROM ImageHaarMatrix "
                            "WHERE matrix IS NOT NULL LIMIT 1;"),
                    &values);

    // return true if there is at least one fingerprint
    return !values.isEmpty();
}

QList<qlonglong> AlbumDB::getDirtyOrMissingFingerprints()
{
    QList<qlonglong> itemIDs;
    QList<QVariant> values;

    d->db->execSql( QString("SELECT id FROM Images "
                            "LEFT JOIN ImageHaarMatrix ON Images.id=ImageHaarMatrix.imageid "
                            " WHERE Images.status=1 AND "
                            " ( ImageHaarMatrix.imageid IS NULL "
                            "   OR Images.modificationDate != ImageHaarMatrix.modificationDate "
                            "   OR Images.uniqueHash != ImageHaarMatrix.uniqueHash ); "),
                    &values );

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

QStringList AlbumDB::getDirtyOrMissingFingerprintURLs()
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
                            "LEFT JOIN ImageHaarMatrix ON Images.id=ImageHaarMatrix.imageid "
                            "LEFT JOIN Albums ON Albums.id=Images.album "
                            " WHERE Images.status=1 AND "
                            " ( ImageHaarMatrix.imageid IS NULL "
                            "   OR Images.modificationDate != ImageHaarMatrix.modificationDate "
                            "   OR Images.uniqueHash != ImageHaarMatrix.uniqueHash ); "),
                    &values );

    QStringList urls;
    QString albumRootPath, relativePath, name;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;
        if (relativePath == "/")
            urls << albumRootPath + relativePath + name;
        else
            urls << albumRootPath + relativePath + '/' + name;
    }

    return urls;
}

QList<ItemScanInfo> AlbumDB::getIdenticalFiles(qlonglong id)
{
    if (!id)
        return QList<ItemScanInfo>();

    QList<QVariant> values;

    // retrieve unique hash and file size
    d->db->execSql( QString("SELECT uniqueHash, fileSize FROM Images WHERE id=?; "),
                    id,
                    &values );

    if (values.isEmpty())
        return QList<ItemScanInfo>();

    QString uniqueHash = values[0].toString();
    int fileSize       = values[1].toInt();

    return getIdenticalFiles(fileSize, uniqueHash, id);
}

QList<ItemScanInfo> AlbumDB::getIdenticalFiles(int fileSize, const QString& uniqueHash, qlonglong sourceId)
{
    // enforce validity
    if (uniqueHash.isEmpty() || fileSize <= 0)
        return QList<ItemScanInfo>();

    QList<QVariant> values;

    // find items with same fingerprint
    d->db->execSql( QString("SELECT id, album, name, status, category, modificationDate FROM Images "
                            " WHERE fileSize=? AND uniqueHash=?; "),
                    fileSize, uniqueHash,
                    &values );

    QList<ItemScanInfo> list;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        ItemScanInfo info;

        info.id               = (*it).toLongLong();
        ++it;
        info.albumID          = (*it).toInt();
        ++it;
        info.itemName         = (*it).toString();
        ++it;
        info.status           = (DatabaseItem::Status)(*it).toInt();
        ++it;
        info.category         = (DatabaseItem::Category)(*it).toInt();
        ++it;
        info.modificationDate = ((*it).isNull() ? QDateTime()
                                    : QDateTime::fromString( (*it).toString(), Qt::ISODate ));
        ++it;

        // exclude one source id from list
        if (sourceId == info.id)
            continue;

        // same for all here, per definition
        info.uniqueHash       = uniqueHash;

        list << info;
    }

    return list;
}

QStringList AlbumDB::imagesFieldList(DatabaseFields::Images fields)
{
    // adds no spaces at beginning or end
    QStringList list;
    if (fields & DatabaseFields::Album)
        list << "album";
    if (fields & DatabaseFields::Name)
        list << "name";
    if (fields & DatabaseFields::Status)
        list << "status";
    if (fields & DatabaseFields::Category)
        list << "category";
    if (fields & DatabaseFields::ModificationDate)
        list << "modificationDate";
    if (fields & DatabaseFields::FileSize)
        list << "fileSize";
    if (fields & DatabaseFields::UniqueHash)
        list << "uniqueHash";

    return list;
}

QStringList AlbumDB::imageInformationFieldList(DatabaseFields::ImageInformation fields)
{
    // adds no spaces at beginning or end
    QStringList list;
    if (fields & DatabaseFields::Rating)
        list << "rating";
    if (fields & DatabaseFields::CreationDate)
        list << "creationDate";
    if (fields & DatabaseFields::DigitizationDate)
        list << "digitizationDate";
    if (fields & DatabaseFields::Orientation)
        list << "orientation";
    if (fields & DatabaseFields::Width)
        list << "width";
    if (fields & DatabaseFields::Height)
        list << "height";
    if (fields & DatabaseFields::Format)
        list << "format";
    if (fields & DatabaseFields::ColorDepth)
        list << "colorDepth";
    if (fields & DatabaseFields::ColorModel)
        list << "colorModel";

    return list;
}

QStringList AlbumDB::imageMetadataFieldList(DatabaseFields::ImageMetadata fields)
{
    // adds no spaces at beginning or end
    QStringList list;
    if (fields & DatabaseFields::Make)
        list << "make";
    if (fields & DatabaseFields::Model)
        list << "model";
    if (fields & DatabaseFields::Lens)
        list << "lens";
    if (fields & DatabaseFields::Aperture)
        list << "aperture";
    if (fields & DatabaseFields::FocalLength)
        list << "focalLength";
    if (fields & DatabaseFields::FocalLength35)
        list << "focalLength35";
    if (fields & DatabaseFields::ExposureTime)
        list << "exposureTime";
    if (fields & DatabaseFields::ExposureProgram)
        list << "exposureProgram";
    if (fields & DatabaseFields::ExposureMode)
        list << "exposureMode";
    if (fields & DatabaseFields::Sensitivity)
        list << "sensitivity";
    if (fields & DatabaseFields::FlashMode)
        list << "flash";
    if (fields & DatabaseFields::WhiteBalance)
        list << "whiteBalance";
    if (fields & DatabaseFields::WhiteBalanceColorTemperature)
        list << "whiteBalanceColorTemperature";
    if (fields & DatabaseFields::MeteringMode)
        list << "meteringMode";
    if (fields & DatabaseFields::SubjectDistance)
        list << "subjectDistance";
    if (fields & DatabaseFields::SubjectDistanceCategory)
        list << "subjectDistanceCategory";

    return list;
}

QStringList AlbumDB::imagePositionsFieldList(DatabaseFields::ImagePositions fields)
{
    // adds no spaces at beginning or end
    QStringList list;
    if (fields & DatabaseFields::Latitude)
        list << "latitude";
    if (fields & DatabaseFields::LatitudeNumber)
        list << "latitudeNumber";
    if (fields & DatabaseFields::Longitude)
        list << "longitude";
    if (fields & DatabaseFields::LongitudeNumber)
        list << "longitudeNumber";
    if (fields & DatabaseFields::Altitude)
        list << "altitude";
    if (fields & DatabaseFields::PositionOrientation)
        list << "orientation";
    if (fields & DatabaseFields::PositionTilt)
        list << "tilt";
    if (fields & DatabaseFields::PositionRoll)
        list << "roll";
    if (fields & DatabaseFields::PositionAccuracy)
        list << "accuracy";
    if (fields & DatabaseFields::PositionDescription)
        list << "description";

    return list;
}

QStringList AlbumDB::imageCommentsFieldList(DatabaseFields::ImageComments fields)
{
    // adds no spaces at beginning or end
    QStringList list;
    if (fields & DatabaseFields::CommentType)
        list << "type";
    if (fields & DatabaseFields::CommentLanguage)
        list << "language";
    if (fields & DatabaseFields::CommentAuthor)
        list << "author";
    if (fields & DatabaseFields::CommentDate)
        list << "date";
    if (fields & DatabaseFields::Comment)
        list << "comment";

    return list;
}


void AlbumDB::addBoundValuePlaceholders(QString& query, int count)
{
    // adds no spaces at beginning or end
    QString questionMarks;
    questionMarks.reserve(count * 2);
    QString questionMark("?,");

    for (int i=0; i<count; ++i)
        questionMarks += questionMark;
    // remove last ','
    questionMarks.chop(1);

    query += questionMarks;
}

int AlbumDB::findInDownloadHistory(const QString& identifier, const QString& name, int fileSize, const QDateTime& date)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id FROM DownloadHistory WHERE "
                            "identifier=? AND filename=? AND filesize=? AND filedate=?;"),
                    identifier, name, fileSize, date.toString(Qt::ISODate), &values);

    if (values.isEmpty())
        return -1;
    return values.first().toInt();
}

int AlbumDB::addToDownloadHistory(const QString& identifier, const QString& name, int fileSize, const QDateTime& date)
{
    QVariant id;
    d->db->execSql( QString("REPLACE INTO DownloadHistory "
                            "(identifier, filename, filesize, filedate) "
                            "VALUES (?,?,?,?);"),
                    identifier, name, fileSize, date.toString(Qt::ISODate), 0, &id);

    return id.toInt();
}



/*
void AlbumDB::setItemCaption(qlonglong imageID,const QString& caption)
{
    QList<QVariant> values;

    d->db->execSql( QString("UPDATE Images SET caption=? "
                            "WHERE id=?;"),
                    caption,
                    imageID );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageComment);
}


void AlbumDB::setItemCaption(int albumID, const QString& name, const QString& caption)
{
    / *
    QList<QVariant> values;

    d->db->execSql( QString("UPDATE Images SET caption=? "
                     "WHERE dirid=? AND name=?;")
             .(caption,
                  QString::number(albumID),
                  (name)) );
    * /

    // easier because of attributes watch
    return setItemCaption(getImageId(albumID, name), caption);
}
*/

void AlbumDB::addItemTag(qlonglong imageID, int tagID)
{
    d->db->execSql( QString("REPLACE INTO ImageTags (imageid, tagid) "
                            "VALUES(?, ?);"),
                    imageID,
                    tagID );

    d->db->recordChangeset(ImageTagChangeset(imageID, tagID, ImageTagChangeset::Added));

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
    d->db->execSql( QString("REPLACE INTO ImageTags (imageid, tagid) \n "
                     "(SELECT id, ? FROM Images \n "
                     " WHERE dirid=? AND name=?);")
             .tagID
             .albumID
             .(name) );
    */

    // easier because of attributes watch
    return addItemTag(getImageId(albumID, name), tagID);
}

void AlbumDB::addTagsToItems(QList<qlonglong> imageIDs, QList<int> tagIDs)
{
    QSqlQuery query = d->db->prepareQuery("REPLACE INTO ImageTags (imageid, tagid) VALUES(?, ?);");

    QVariantList images;
    QVariantList tags;

    foreach (const qlonglong& imageid, imageIDs)
    {
        foreach (int tagid, tagIDs)
        {
            images << imageid;
            tags << tagid;
        }
    }

    query.addBindValue(images);
    query.addBindValue(tags);
    d->db->execBatch(query);
    d->db->recordChangeset(ImageTagChangeset(imageIDs, tagIDs, ImageTagChangeset::Added));
}

QList<int> AlbumDB::getRecentlyAssignedTags() const
{
    return d->recentlyAssignedTags;
}

void AlbumDB::removeItemTag(qlonglong imageID, int tagID)
{
    d->db->execSql( QString("DELETE FROM ImageTags "
                            "WHERE imageID=? AND tagid=?;"),
                    imageID,
                    tagID );

    d->db->recordChangeset(ImageTagChangeset(imageID, tagID, ImageTagChangeset::Removed));
}

void AlbumDB::removeItemAllTags(qlonglong imageID)
{
    d->db->execSql( QString("DELETE FROM ImageTags "
                            "WHERE imageID=?;"),
                    imageID );

    d->db->recordChangeset(ImageTagChangeset(imageID, QList<int>(), ImageTagChangeset::RemovedAll));
}

void AlbumDB::removeTagsFromItems(QList<qlonglong> imageIDs, QList<int> tagIDs)
{
    QSqlQuery query = d->db->prepareQuery("DELETE FROM ImageTags WHERE imageID=? AND tagid=?;");

    QVariantList images;
    QVariantList tags;

    foreach (const qlonglong& imageid, imageIDs)
    {
        foreach (int tagid, tagIDs)
        {
            images << imageid;
            tags << tagid;
        }
    }

    query.addBindValue(images);
    query.addBindValue(tags);
    d->db->execBatch(query);
    d->db->recordChangeset(ImageTagChangeset(imageIDs, tagIDs, ImageTagChangeset::Removed));
}

QStringList AlbumDB::getItemNamesInAlbum(int albumID, bool recurssive)
{
    QList<QVariant> values;

    if (recurssive)
    {
        KUrl url(getAlbumRelativePath(albumID));
        int rootId = getAlbumRootId(albumID);
        QString path = url.path();
        d->db->execSql( QString("SELECT Images.name FROM Images WHERE Images.album IN "
                                " (SELECT DISTINCT id FROM Albums "
                                "  WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?));"),
                        rootId, path, path == "/" ? "/%" : path + "/%",
                        &values );
    }
    else
    {
        d->db->execSql( QString("SELECT Images.name "
                                "FROM Images "
                                "WHERE Images.album=?"),
                        albumID, &values );
    }

    QStringList names;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
        names << it->toString();
    return names;
}

/*
QStringList AlbumDB::getAllItemURLsWithoutDate()
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
                            "FROM Images "
                            "  LEFT JOIN Albums ON Images.album=Albums.id "
                            "  LEFT JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                            "WHERE (Images.datetime is null or "
                            "       Images.datetime == '');"),
                    &values );

    QStringList urls;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        urls << it->toString();

    return urls;
}
*/

QList<QDateTime> AlbumDB::getAllCreationDates()
{
    QList<QVariant> values;
    d->db->execSql( "SELECT creationDate FROM ImageInformation "
                    " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                    " WHERE Images.status=1;", &values );

    QList<QDateTime> list;
    foreach (const QVariant& value, values)
    {
        if (!value.isNull())
            list << QDateTime::fromString(value.toString(), Qt::ISODate);
    }
    return list;
}

QMap<QDateTime, int> AlbumDB::getAllCreationDatesAndNumberOfImages()
{
    QList<QVariant> values;
    d->db->execSql( "SELECT creationDate FROM ImageInformation "
                    " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                    " WHERE Images.status=1;", &values );

    QMap<QDateTime, int> datesStatMap;
    foreach (const QVariant& value, values)
    {
        if (!value.isNull())
        {
            QDateTime dateTime = QDateTime::fromString(value.toString(), Qt::ISODate);
            if ( !dateTime.isValid() )
                continue;

            QMap<QDateTime, int>::iterator it2 = datesStatMap.find(dateTime);
            if ( it2 == datesStatMap.end() )
                datesStatMap.insert( dateTime, 1 );
            else
                it2.value()++;
        }
    }
    return datesStatMap;
}

QMap<int, int> AlbumDB::getNumberOfImagesInAlbums()
{
    QList<QVariant> values, allAbumIDs;
    QMap<int, int>  albumsStatMap;
    int             albumID;

    // initialize allAbumIDs with all existing albums from db to prevent
    // wrong album image counters
    d->db->execSql("SELECT id from Albums", &allAbumIDs);

    for (QList<QVariant>::const_iterator it = allAbumIDs.constBegin(); it != allAbumIDs.constEnd(); ++it)
    {
        albumID = (*it).toInt();
        albumsStatMap.insert(albumID, 0);
    }

    d->db->execSql( "SELECT album FROM Images WHERE Images.status=1;", &values );

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumID = (*it).toInt();
        ++it;

        QMap<int, int>::iterator it2 = albumsStatMap.find(albumID);
        if ( it2 == albumsStatMap.end() )
            albumsStatMap.insert(albumID, 1);
        else
            it2.value()++;
    }

    return albumsStatMap;
}

QMap<int, int> AlbumDB::getNumberOfImagesInTags()
{
    QList<QVariant> values, allTagIDs;
    QMap<int, int>  tagsStatMap;
    int             tagID;

    // initialize allTagIDs with all existing tags from db to prevent
    // wrong tag counters
    d->db->execSql(QString("SELECT id from Tags"), &allTagIDs);

    for (QList<QVariant>::const_iterator it = allTagIDs.constBegin(); it != allTagIDs.constEnd(); ++it)
    {
        tagID = (*it).toInt();
        tagsStatMap.insert(tagID, 0);
    }

    d->db->execSql( "SELECT tagid FROM ImageTags "
                    " LEFT JOIN Images ON Images.id=ImageTags.imageid "
                    " WHERE Images.status=1;", &values );

    for (QList<QVariant>::const_iterator it=values.constBegin(); it != values.constEnd();)
    {
        tagID = (*it).toInt();
        ++it;

        QMap<int, int>::iterator it2 = tagsStatMap.find(tagID);
        if ( it2 == tagsStatMap.end() )
            tagsStatMap.insert(tagID, 1);
        else
            it2.value()++;
    }

    return tagsStatMap;
}

QMap<QString,int> AlbumDB::getImageFormatStatistics()
{
    QMap<QString, int>  map;

    QSqlQuery query;
    query = d->db->prepareQuery("SELECT COUNT(*), II.format "
                                "FROM ImageInformation AS II "
                                "   INNER JOIN Images ON II.imageid=images.id "
                                "WHERE Images.status=1 "
                                "GROUP BY II.format;");
    if (d->db->exec(query))
    {
        while (query.next())
        {
            QString quantity = query.value(0).toString();
            QString format   = query.value(1).toString();
            if (format.isEmpty())
                continue;
            map[format] = quantity.isEmpty() ? 0 : quantity.toInt();
        }
    }

    return map;
}

/*
QList<QPair<QString, QDateTime> > AlbumDB::getItemsAndDate()
{
    QList<QVariant> values;
    d->db->execSql( "SELECT Images.name, datetime FROM Images;", &values );

    QList<QPair<QString, QDateTime> > data;
    for ( QList<QVariant>::iterator it = values.begin(); it != values.end(); )
    {
        QPair<QString, QDateTime> pair;
        pair.first  = (*it).toString();
        ++it;
        pair.second = QDateTime::fromString( (*it).toString(),  Qt::ISODate );
        ++it;

        if (!pair.second.isValid())
            continue;

        data << pair;
    }
    return data;
}
*/

/*
int AlbumDB::getAlbumForPath(const QString& albumRoot, const QString& folder, bool create)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);
    if (location.isNull())
        return -1;

    return getAlbumForPath(location.id(), folder, create);

}
*/

int AlbumDB::getAlbumForPath(int albumRootId, const QString& folder, bool create)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id FROM Albums WHERE albumRoot=? AND relativePath=?;"),
                    albumRootId, folder, &values);

    int albumID = -1;
    if (values.isEmpty())
    {
        if (create)
            albumID = addAlbum(albumRootId, folder, QString(), QDate::currentDate(), QString());
    }
    else
    {
        albumID = values.first().toInt();
    }

    return albumID;
}

QList<int> AlbumDB::getAlbumAndSubalbumsForPath(int albumRootId, const QString& relativePath)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id FROM Albums WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?);"),
                    albumRootId, relativePath, (relativePath == "/" ? "/%" : relativePath + "/%"), &values);

    QList<int> albumIds;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        albumIds << (*it).toInt();
    }
    return albumIds;
}

QList<int> AlbumDB::getAlbumsOnAlbumRoot(int albumRootId)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id FROM Albums WHERE albumRoot=?;"),
                    albumRootId, &values);

    QList<int> albumIds;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        albumIds << (*it).toInt();
    }
    return albumIds;
}

qlonglong AlbumDB::addItem(int albumID, const QString& name,
                           DatabaseItem::Status status,
                           DatabaseItem::Category category,
                           const QDateTime& modificationDate,
                           int fileSize,
                           const QString& uniqueHash)
{
    QVariantList boundValues;
    boundValues << albumID << name << (int)status << (int)category
                << modificationDate.toString(Qt::ISODate) << fileSize << uniqueHash;

    QVariant id;
    d->db->execSql ( QString ("REPLACE INTO Images "
                              " ( album, name, status, category, modificationDate, fileSize, uniqueHash ) "
                              " VALUES (?,?,?,?,?,?,?);" ),
                     boundValues,
                     0, &id);

    if (id.isNull())
        return -1;
    d->db->recordChangeset(ImageChangeset(id.toLongLong(), DatabaseFields::ImagesAll));
    d->db->recordChangeset(CollectionImageChangeset(id.toLongLong(), albumID, CollectionImageChangeset::Added));
    return id.toLongLong();
}

void AlbumDB::updateItem(qlonglong imageID, DatabaseItem::Category category,
                         const QDateTime& modificationDate,
                         int fileSize, const QString& uniqueHash)
{
    QVariantList boundValues;
    boundValues << category << modificationDate << fileSize << uniqueHash << imageID;
    d->db->execSql( QString("UPDATE Images SET category=?, modificationDate=?, fileSize=?, uniqueHash=? WHERE id=?;"),
                    boundValues );
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Category
                                                 | DatabaseFields::ModificationDate
                                                 | DatabaseFields::FileSize
                                                 | DatabaseFields::UniqueHash ));
}

QList<int> AlbumDB::getTagsFromTagPaths(const QStringList& keywordsList, bool create)
{
    if (keywordsList.isEmpty())
        return QList<int>();

    QList<int> tagIDs;

    QStringList keywordsList2Create;

    // Create a list of the tags currently in database

    TagInfo::List currentTagsList;

    QList<QVariant> values;
    d->db->execSql( "SELECT id, pid, name FROM Tags;", &values );

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        TagInfo info;

        info.id   = (*it).toInt();
        ++it;
        info.pid  = (*it).toInt();
        ++it;
        info.name = (*it).toString();
        ++it;
        currentTagsList.append(info);
    }

    // For every tag in keywordsList, scan taglist to check if tag already exists.

    for (QStringList::const_iterator kwd = keywordsList.constBegin();
        kwd != keywordsList.constEnd(); ++kwd )
    {
        // split full tag "url" into list of single tag names
        QStringList tagHierarchy = (*kwd).split('/', QString::SkipEmptyParts);
        if (tagHierarchy.isEmpty())
            continue;

        // last entry in list is the actual tag name
        bool foundTag   = false;
        QString tagName = tagHierarchy.back();
        tagHierarchy.pop_back();

        for (TagInfo::List::const_iterator tag = currentTagsList.constBegin();
            tag != currentTagsList.constEnd(); ++tag )
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

                    for (TagInfo::List::const_iterator parentTag = currentTagsList.constBegin();
                        parentTag != currentTagsList.constEnd(); ++parentTag )
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
        for (QStringList::const_iterator kwd = keywordsList2Create.constBegin();
            kwd != keywordsList2Create.constEnd(); ++kwd )
        {
            // split full tag "url" into list of single tag names
            QStringList tagHierarchy = (*kwd).split('/', QString::SkipEmptyParts);

            if (tagHierarchy.isEmpty())
                continue;

            int  parentTagID      = 0;
            int  tagID            = 0;
            bool parentTagExisted = true;

            // Traverse hierarchy from top to bottom
            for (QStringList::const_iterator tagName = tagHierarchy.constBegin();
                tagName != tagHierarchy.constEnd(); ++tagName)
            {
                tagID = 0;

                // if the parent tag did not exist, we need not check if the child exists
                if (parentTagExisted)
                {
                    for (TagInfo::List::const_iterator tag = currentTagsList.constBegin();
                        tag != currentTagsList.constEnd(); ++tag )
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

int AlbumDB::getItemAlbum(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT album FROM Images WHERE id=?;"),
                   imageID, &values);

    if (!values.isEmpty())
        return values.first().toInt();
    else
        return 1;
}

QString AlbumDB::getItemName(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT name FROM Images WHERE id=?;"),
                   imageID, &values);

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

/*
bool AlbumDB::setItemDate(qlonglong imageID,
                          const QDateTime& datetime)
{
    d->db->execSql ( QString ("UPDATE Images SET datetime=?"
                            "WHERE id=?;"),
                     datetime.toString(Qt::ISODate),
                     imageID );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageDate);

    return true;
}

bool AlbumDB::setItemDate(int albumID, const QString& name,
                          const QDateTime& datetime)
{
    / *
    d->db->execSql ( QString ("UPDATE Images SET datetime=?"
                       "WHERE dirid=? AND name=?;")
              .datetime.toString(Qt::ISODate,
                   QString::number(albumID),
                   (name)) );

    return true;
    * /
    // easier because of attributes watch
    return setItemDate(getImageId(albumID, name), datetime);
}


void AlbumDB::setItemRating(qlonglong imageID, int rating)
{
    d->db->execSql ( QString ("REPLACE INTO ImageProperties "
                            "(imageid, property, value) "
                            "VALUES(?, ?, ?);"),
                     imageID,
                     QString("Rating"),
                     rating );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageRating);
}

int AlbumDB::getItemRating(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT value FROM ImageProperties "
                            "WHERE imageid=? and property=?;"),
                    imageID,
                    QString("Rating"),
                    &values);

    if (!values.isEmpty())
        return values.first().toInt();
    else
        return 0;
}
*/

QStringList AlbumDB::getItemURLsInAlbum(int albumID, ItemSortOrder sortOrder)
{
    QList<QVariant> values;

    int albumRootId = getAlbumRootId(albumID);
    if (albumRootId == -1)
        return QStringList();
    QString albumRootPath = CollectionManager::instance()->albumRootPath(albumRootId);
    if (albumRootPath.isNull())
        return QStringList();

    QString sqlString;
    switch(sortOrder)
    {
        case ByItemName:
            sqlString = QString("SELECT Albums.relativePath, Images.name "
                                 "FROM Images INNER JOIN Albums ON Albums.id=Images.album "
                                 "WHERE Albums.id=? "
                                 "ORDER BY Images.name COLLATE NOCASE;");
            break;
        case ByItemPath:
            // Don't collate on the path - this is to maintain the same behavior
            // that happens when sort order is "By Path"
            sqlString = QString("SELECT Albums.relativePath, Images.name "
                                 "FROM Images INNER JOIN Albums ON Albums.id=Images.album "
                                 "WHERE Albums.id=? "
                                 "ORDER BY Albums.relativePath,Images.name;");
            break;
        case ByItemDate:
            sqlString = QString("SELECT Albums.relativePath, Images.name "
                                 "FROM Images INNER JOIN Albums ON Albums.id=Images.album "
                                 "            INNER JOIN ImageInformation ON ImageInformation.imageid=Images.id "
                                 "WHERE Albums.id=? "
                                 "ORDER BY ImageInformation.creationDate;");
            break;
        case ByItemRating:
            sqlString = QString("SELECT Albums.relativePath, Images.name "
                                 "FROM Images INNER JOIN Albums ON Albums.id=Images.album "
                                 "            INNER JOIN ImageInformation ON ImageInformation.imageid=Images.id "
                                 "WHERE Albums.id=? "
                                 "ORDER BY ImageInformation.rating DESC;");
            break;
        case NoItemSorting:
        default:
            sqlString = QString("SELECT Albums.relativePath, Images.name "
                                 "FROM Images INNER JOIN Albums ON Albums.id=Images.album "
                                 "WHERE Albums.id=?;");
            break;
    }
    // all statements take one bound value
    d->db->execSql(sqlString, albumID, &values);

    QStringList urls;
    QString relativePath, name;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;
        if (relativePath == "/")
            urls << albumRootPath + relativePath + name;
        else
            urls << albumRootPath + relativePath + '/' + name;
    }

    return urls;
}

QList<qlonglong> AlbumDB::getItemIDsInAlbum(int albumID)
{
    QList<qlonglong> itemIDs;
    QList<QVariant> values;

    d->db->execSql( QString("SELECT id FROM Images WHERE album=?;"),
             albumID, &values );

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

QMap<qlonglong, QString> AlbumDB::getItemIDsAndURLsInAlbum(int albumID)
{
    int albumRootId = getAlbumRootId(albumID);
    if (albumRootId == -1)
        return QMap<qlonglong, QString>();

    QString albumRootPath = CollectionManager::instance()->albumRootPath(albumRootId);
    if (albumRootPath.isNull())
        return QMap<qlonglong, QString>();

    QMap<qlonglong, QString> itemsMap;
    QList<QVariant> values;

    d->db->execSql( QString("SELECT Images.id, Albums.relativePath, Images.name "
                            "FROM Images JOIN Albums ON Albums.id=Images.album "
                            "WHERE Albums.id=?;"),
                    albumID, &values );

    QString   path;
    qlonglong id;
    QString relativePath, name;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        id = (*it).toLongLong();
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;
        if (relativePath == "/")
            path = albumRootPath + relativePath + name;
        else
            path = albumRootPath + relativePath + '/' + name;

        itemsMap.insert(id, path);
    };

    return itemsMap;
}

QList<ItemScanInfo> AlbumDB::getItemScanInfos(int albumID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT id, album, name, status, category, modificationDate, uniqueHash "
                            "FROM Images WHERE album=?;"),
                    albumID,
                    &values );

    QList<ItemScanInfo> list;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        ItemScanInfo info;

        info.id               = (*it).toLongLong();
        ++it;
        info.albumID          = (*it).toInt();
        ++it;
        info.itemName         = (*it).toString();
        ++it;
        info.status           = (DatabaseItem::Status)(*it).toInt();
        ++it;
        info.category         = (DatabaseItem::Category)(*it).toInt();
        ++it;
        info.modificationDate = ((*it).isNull() ? QDateTime()
                                    : QDateTime::fromString( (*it).toString(), Qt::ISODate ));
        ++it;
        info.uniqueHash       = (*it).toString();
        ++it;

        list << info;
    }

    return list;
}

ItemScanInfo AlbumDB::getItemScanInfo(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT id, album, name, status, category, modificationDate, uniqueHash "
                            "FROM Images WHERE id=?;"),
                    imageID,
                    &values );

    ItemScanInfo info;

    if (!values.isEmpty())
    {
        QList<QVariant>::const_iterator it = values.constBegin();

        info.id               = (*it).toLongLong();
        ++it;
        info.albumID          = (*it).toInt();
        ++it;
        info.itemName         = (*it).toString();
        ++it;
        info.status           = (DatabaseItem::Status)(*it).toInt();
        ++it;
        info.category         = (DatabaseItem::Category)(*it).toInt();
        ++it;
        info.modificationDate = ((*it).isNull() ? QDateTime()
            : QDateTime::fromString( (*it).toString(), Qt::ISODate ));
        ++it;
        info.uniqueHash       = (*it).toString();
        ++it;
    }

    return info;
}

QStringList AlbumDB::getItemURLsInTag(int tagID, bool recursive)
{
    QList<QVariant> values;

    QString imagesIdClause;
    QList<QVariant> boundValues;
    if (recursive)
    {
        imagesIdClause = QString("SELECT imageid FROM ImageTags "
                                 " WHERE tagid=? "
                                 " OR tagid IN (SELECT id FROM TagsTree WHERE pid=?)");
        boundValues << tagID << tagID;
    }
    else
    {
        imagesIdClause = QString("SELECT imageid FROM ImageTags WHERE tagid=?");
        boundValues << tagID;
    }

    d->db->execSql( QString("SELECT Albums.albumRoot, Albums.relativePath, Images.name "
                            "FROM Images JOIN Albums ON Albums.id=Images.album "
                            "WHERE Images.status=1 AND Images.id IN (%1);")
                    .arg(imagesIdClause), boundValues, &values );

    QStringList urls;
    QString albumRootPath, relativePath, name;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;
        if (relativePath == "/")
            urls << albumRootPath + relativePath + name;
        else
            urls << albumRootPath + relativePath + '/' + name;
    }

    return urls;
}

QList<qlonglong> AlbumDB::getItemIDsInTag(int tagID, bool recursive)
{
    QList<qlonglong> itemIDs;
    QList<QVariant> values;

    if (recursive)
        d->db->execSql( QString("SELECT imageid FROM ImageTags JOIN Images ON ImageTags.imageid=Images.id "
                                " WHERE Images.status=1 AND "
                                " ( tagid=? "
                                "   OR tagid IN (SELECT id FROM TagsTree WHERE pid=?) );"),
                        tagID, tagID, &values );
    else
        d->db->execSql( QString("SELECT imageid FROM ImageTags JOIN Images ON ImageTags.imageid=Images.id "
                                " WHERE Images.status=1 AND tagid=?;"),
                 tagID, &values );

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

/*
QString AlbumDB::getAlbumPath(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT AlbumRoots.absolutePath||Albums.relativePath "
                            "FROM Albums, AlbumRoots WHERE Albums.id=? AND AlbumRoots.id=Albums.albumRoot; "),
                    albumID, &values);
    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}
*/

QString AlbumDB::getAlbumRelativePath(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT relativePath from Albums WHERE id=?"),
                    albumID, &values);
    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

int AlbumDB::getAlbumRootId(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT albumRoot FROM Albums WHERE id=?; "),
                    albumID, &values);
    if (!values.isEmpty())
        return values.first().toInt();
    else
        return -1;
}

QDate AlbumDB::getAlbumLowestDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( "SELECT MIN(creationDate) FROM ImageInformation "
                    " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                    " WHERE Images.album=? GROUP BY Images.album;",
                    albumID, &values );
    if (!values.isEmpty())
        return QDate::fromString( values.first().toString(), Qt::ISODate );
    else
        return QDate();
}

QDate AlbumDB::getAlbumHighestDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( "SELECT MAX(creationDate) FROM ImageInformation "
                    " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                    " WHERE Images.album=? GROUP BY Images.album;",
                    albumID , &values );
    if (!values.isEmpty())
        return QDate::fromString( values.first().toString(), Qt::ISODate );
    else
        return QDate();
}

QDate AlbumDB::getAlbumAverageDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( "SELECT creationDate FROM ImageInformation "
                    " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                    " WHERE Images.album=?;",
                    albumID , &values);

    int differenceInSecs = 0;
    int amountOfImages = 0;
    QDateTime baseDateTime;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        QDateTime itemDateTime = (*it).isNull() ? QDateTime()
            : QDateTime::fromString( (*it).toString(), Qt::ISODate );
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
    qlonglong imageId = getImageId(albumID, file);

    d->db->execSql( QString("DELETE FROM Images WHERE id=?;"),
                    imageId );

    d->db->recordChangeset(CollectionImageChangeset(imageId, albumID, CollectionImageChangeset::Deleted));
}

void AlbumDB::removeItemsFromAlbum(int albumID)
{
    d->db->execSql( QString("UPDATE Images SET status=?, album=NULL WHERE album=?;"),
                    (int)DatabaseItem::Removed, albumID );

    d->db->recordChangeset(CollectionImageChangeset(QList<qlonglong>(), albumID, CollectionImageChangeset::RemovedAll));
}

void AlbumDB::removeItems(QList<qlonglong> itemIDs, QList<int> albumIDs)
{
    QSqlQuery query = d->db->prepareQuery( QString("UPDATE Images SET status=?, album=NULL WHERE id=?;") );

    QVariantList imageIds;
    QVariantList status;
    foreach (const qlonglong& id, itemIDs)
    {
        status << (int)DatabaseItem::Removed;
        imageIds << id;
    }

    query.addBindValue(status);
    query.addBindValue(imageIds);
    d->db->execBatch(query);

    d->db->recordChangeset(CollectionImageChangeset(itemIDs, albumIDs, CollectionImageChangeset::Removed));
}

void AlbumDB::deleteRemovedItems()
{
    d->db->execSql( QString("DELETE FROM Images WHERE status=?;"),
                    (int)DatabaseItem::Removed );

    d->db->recordChangeset(CollectionImageChangeset(QList<qlonglong>(), QList<int>(), CollectionImageChangeset::RemovedDeleted));
}

void AlbumDB::deleteRemovedItems(QList<int> albumIds)
{
    QSqlQuery query = d->db->prepareQuery( QString("DELETE FROM Images WHERE status=? AND album=?;") );

    QVariantList albumBindIds;
    QVariantList status;
    foreach(int albumId, albumIds)
    {
        status << (int)DatabaseItem::Removed;
        albumBindIds << albumId;
    }

    query.addBindValue(status);
    query.addBindValue(albumBindIds);
    d->db->execBatch(query);

    d->db->recordChangeset(CollectionImageChangeset(QList<qlonglong>(), albumIds, CollectionImageChangeset::RemovedDeleted));
}

void AlbumDB::renameAlbum(int albumID, int newAlbumRoot, const QString& newRelativePath)
{
    int albumRoot  = getAlbumRootId(albumID);
    QString oldUrl = getAlbumRelativePath(albumID);

    if (oldUrl == newRelativePath)
        return;

    // first delete any stale albums left behind
    d->db->execSql( QString("DELETE FROM Albums WHERE relativePath=? AND albumRoot=?;"),
                    newRelativePath, albumRoot );

    // now update the album
    d->db->execSql( QString("UPDATE Albums SET albumRoot=?, relativePath=? WHERE id=? AND albumRoot=?;"),
                    newAlbumRoot, newRelativePath, albumID, albumRoot );
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Renamed));

    /*
    if (renameSubalbums)
    {
        // now find the list of all subalbums which need to be updated
        QList<QVariant> values;
        d->db->execSql( QString("SELECT id, relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE ?;"),
                        albumRoot, oldUrl + "/%", &values );

        // and update their url
        QString newChildURL;
        for (QList<QVariant>::iterator it = values.begin(); it != values.end(); )
        {
            int childAlbumId = (*it).toInt();
            ++it;
            newChildURL = (*it).toString();
            ++it;
            newChildURL.replace(oldUrl, newRelativePath);
            d->db->execSql(QString("UPDATE Albums SET albumRoot=?, relativePath=? WHERE albumRoot=? AND relativePath=?"),
                           newAlbumRoot, newChildURL, albumRoot, (*it) );
            d->db->recordChangeset(AlbumChangeset(childAlbumId, AlbumChangeset::Renamed));
        }
    }
    */
}

void AlbumDB::setTagName(int tagID, const QString& name)
{
    d->db->execSql( QString("UPDATE Tags SET name=? WHERE id=?;"),
                    name, tagID );
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Renamed));
}

void AlbumDB::moveItem(int srcAlbumID, const QString& srcName,
                       int dstAlbumID, const QString& dstName)
{
    // find id of src image
    qlonglong imageId = getImageId(srcAlbumID, srcName);

    if (imageId == -1)
        return;

    // first delete any stale database entries (for destination) if any
    deleteItem(dstAlbumID, dstName);

    d->db->execSql( QString("UPDATE Images SET album=?, name=? "
                            "WHERE id=?;"),
                    dstAlbumID, dstName, imageId );
    d->db->recordChangeset(CollectionImageChangeset(imageId, srcAlbumID, CollectionImageChangeset::Moved));
    d->db->recordChangeset(CollectionImageChangeset(imageId, srcAlbumID, CollectionImageChangeset::Removed));
    d->db->recordChangeset(CollectionImageChangeset(imageId, dstAlbumID, CollectionImageChangeset::Added));
}

int AlbumDB::copyItem(int srcAlbumID, const QString& srcName,
                      int dstAlbumID, const QString& dstName)
{
    // find id of src image
    qlonglong srcId = getImageId(srcAlbumID, srcName);

    if (srcId == -1 || dstAlbumID == -1 || dstName.isEmpty())
        return -1;

    // check for src == dest
    if (srcAlbumID == dstAlbumID && srcName == dstName)
        return srcId;

    // first delete any stale database entries if any
    deleteItem(dstAlbumID, dstName);

    // copy entry in Images table
    QVariant id;
    d->db->execSql ( QString ("INSERT INTO Images "
                              " ( album, name, status, category, modificationDate, fileSize, uniqueHash ) "
                              " SELECT ?, ?, status, category, modificationDate, fileSize, uniqueHash "
                              "  FROM Images WHERE id=?;"),
                     dstAlbumID, dstName, srcId,
                     0, &id);

    if (id.isNull())
        return -1;

    d->db->recordChangeset(ImageChangeset(id.toLongLong(), DatabaseFields::ImagesAll));
    d->db->recordChangeset(CollectionImageChangeset(id.toLongLong(), srcAlbumID, CollectionImageChangeset::Copied));
    d->db->recordChangeset(CollectionImageChangeset(id.toLongLong(), dstAlbumID, CollectionImageChangeset::Added));

    // copy all other tables
    copyImageAttributes(srcId, id.toLongLong());

    return id.toLongLong();
}

void AlbumDB::copyImageAttributes(qlonglong srcId, qlonglong dstId)
{
    // Go through all image-specific tables and copy the entries

    DatabaseFields::Set fields;

    d->db->execSql( QString("INSERT INTO ImageHaarMatrix "
                            " (imageid, modificationDate, uniqueHash, matrix) "
                            "SELECT ?, modificationDate, uniqueHash, matrix "
                            "FROM ImageHaarMatrix WHERE imageid=?;"),
                    dstId, srcId );

    d->db->execSql( QString("INSERT INTO ImageInformation "
                            " (imageid, rating, creationDate, digitizationDate, orientation, "
                            "  width, height, format, colorDepth, colorModel) "
                            "SELECT ?, rating, creationDate, digitizationDate, orientation, "
                            "  width, height, format, colorDepth, colorModel "
                            "FROM ImageInformation WHERE imageid=?;"),
                    dstId, srcId );
    fields |= DatabaseFields::ImageInformationAll;

    d->db->execSql( QString("INSERT INTO ImageMetadata "
                            " (imageid, make, model, lens, aperture, focalLength, focalLength35, "
                            "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                            "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) "
                            "SELECT ?, make, model, lens, aperture, focalLength, focalLength35, "
                            "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                            "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory "
                            "FROM ImageMetadata WHERE imageid=?;"),
                    dstId, srcId );
    fields |= DatabaseFields::ImageMetadataAll;

    d->db->execSql( QString("INSERT INTO ImagePositions "
                            " (imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                            "  altitude, orientation, tilt, roll, accuracy, description) "
                            "SELECT ?, latitude, latitudeNumber, longitude, longitudeNumber, "
                            "  altitude, orientation, tilt, roll, accuracy, description "
                            "FROM ImagePositions WHERE imageid=?;"),
                    dstId, srcId );
    fields |= DatabaseFields::ImagePositionsAll;

    d->db->execSql( QString("INSERT INTO ImageComments "
                            " (imageid, type, language, author, date, comment) "
                            "SELECT ?, type, language, author, date, comment "
                            "FROM ImageComments WHERE imageid=?;"),
                    dstId, srcId );
    fields |= DatabaseFields::ImageCommentsAll;

    d->db->execSql( QString("INSERT INTO ImageCopyright "
                            " (imageid, property, value, extraValue) "
                            "SELECT ?, property, value, extraValue "
                            "FROM ImageCopyright WHERE imageid=?;"),
                    dstId, srcId );

    d->db->recordChangeset(ImageChangeset(dstId, fields));

    d->db->execSql( QString("INSERT INTO ImageTags "
                            " (imageid, tagid) "
                            "SELECT ?, tagid "
                            "FROM ImageTags WHERE imageid=?;"),
                    dstId, srcId );
    // leave empty tag list for now
    d->db->recordChangeset(ImageTagChangeset(dstId, QList<int>(), ImageTagChangeset::Added));

    d->db->execSql( QString("INSERT INTO ImageProperties "
                            " (imageid, property, value) "
                            "SELECT ?, property, value "
                            "FROM ImageProperties WHERE imageid=?;"),
                    dstId, srcId );
}

bool AlbumDB::copyAlbumProperties(int srcAlbumID, int dstAlbumID)
{
    if (srcAlbumID == dstAlbumID)
        return true;

    QList<QVariant> values;
    d->db->execSql( QString("SELECT date, caption, collection, icon "
                            "FROM Albums WHERE id=?;"),
                    srcAlbumID,
                    &values );

    if (values.isEmpty())
    {
        kWarning(50003) << " src album ID " << srcAlbumID << " does not exist";
        return false;
    }

    QList<QVariant> boundValues;
    boundValues << values[0] << values[1] << values[2] << values[3];
    boundValues << dstAlbumID;

    d->db->execSql( QString("UPDATE Albums SET date=?, caption=?, "
                            "collection=?, icon=? WHERE id=?"),
                    boundValues );
    return true;
}

}  // namespace Digikam
