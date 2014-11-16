/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description :database album interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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
#include <QVariant>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "databasebackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "dbactiontype.h"
#include "tagscache.h"

namespace Digikam
{

class AlbumDB::Private
{

public:

    Private() :
        db(0),
        uniqueHashVersion(-1)
    {
    }

    static const QString configGroupName;
    static const QString configRecentlyUsedTags;

    DatabaseBackend*     db;
    QList<int>           recentlyAssignedTags;

    int                  uniqueHashVersion;

public:

    QString constructRelatedImagesSQL(bool fromOrTo, DatabaseRelation::Type type, bool boolean);
    QList<qlonglong> execRelatedImagesQuery(SqlQuery& query, qlonglong id, DatabaseRelation::Type type);
};

const QString AlbumDB::Private::configGroupName("AlbumDB Settings");
const QString AlbumDB::Private::configRecentlyUsedTags("Recently Used Tags");

QString AlbumDB::Private::constructRelatedImagesSQL(bool fromOrTo, DatabaseRelation::Type type, bool boolean)
{
    QString sql;

    if (fromOrTo)
    {
        sql = "SELECT object FROM ImageRelations "
              "INNER JOIN Images ON ImageRelations.object=Images.id "
              "WHERE subject=? %1 AND status!=3 %2;";
    }
    else
    {
        sql = "SELECT subject FROM ImageRelations "
              "INNER JOIN Images ON ImageRelations.subject=Images.id "
              "WHERE object=? %1 AND status!=3 %2;";
    }

    if (type != DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString("AND type=?"));
    }
    else
    {
        sql = sql.arg(QString());
    }

    if (boolean)
    {
        sql = sql.arg(QString("LIMIT 1"));
    }
    else
    {
        sql = sql.arg(QString());
    }

    return sql;
}

QList<qlonglong> AlbumDB::Private::execRelatedImagesQuery(SqlQuery& query, qlonglong id, DatabaseRelation::Type type)
{
    QVariantList values;

    if (type == DatabaseRelation::UndefinedType)
    {
        db->execSql(query, id, &values);
    }
    else
    {
        db->execSql(query, id, type, &values);
    }

    QList<qlonglong> imageIds;

    if (values.isEmpty())
    {
        return imageIds;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        imageIds << (*it).toInt();
    }

    return imageIds;
}

// --------------------------------------------------------

AlbumDB::AlbumDB(DatabaseBackend* const backend)
    : d(new Private)
{
    d->db = backend;
    readSettings();
}

AlbumDB::~AlbumDB()
{
    writeSettings();
    delete d;
}

QList<AlbumRootInfo> AlbumDB::getAlbumRoots()
{
    QList<AlbumRootInfo> list;
    QList<QVariant>      values;

    d->db->execSql("SELECT id, label, status, type, identifier, specificPath FROM AlbumRoots;", &values);

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
    d->db->execSql(QString("REPLACE INTO AlbumRoots (type, label, status, identifier, specificPath) "
                           "VALUES(?, ?, 0, ?, ?);"),
                   (int)type, label, identifier, specificPath, 0, &id);

    d->db->recordChangeset(AlbumRootChangeset(id.toInt(), AlbumRootChangeset::Added));
    return id.toInt();
}

void AlbumDB::deleteAlbumRoot(int rootId)
{
    d->db->execSql(QString("DELETE FROM AlbumRoots WHERE id=?;"),
                   rootId);
    QMap<QString, QVariant> parameters;
    parameters.insert(":albumRoot", rootId);

    if (DatabaseCoreBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QString("deleteAlbumRoot")), parameters))
    {
        return;
    }

    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::Deleted));
}

void AlbumDB::migrateAlbumRoot(int rootId, const QString& identifier)
{
    d->db->execSql(QString("UPDATE AlbumRoots SET identifier=? WHERE id=?;"),
                   identifier, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

void AlbumDB::setAlbumRootLabel(int rootId, const QString& newLabel)
{
    d->db->execSql(QString("UPDATE AlbumRoots SET label=? WHERE id=?;"),
                   newLabel, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

void AlbumDB::changeAlbumRootType(int rootId, AlbumRoot::Type newType)
{
    d->db->execSql(QString("UPDATE AlbumRoots SET type=? WHERE id=?;"),
                   (int)newType, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

AlbumInfo::List AlbumDB::scanAlbums()
{
    AlbumInfo::List aList;

    QList<QVariant> values;
    d->db->execSql("SELECT albumRoot, id, relativePath, date, caption, collection, icon FROM Albums "
                   " WHERE albumRoot != 0;", // exclude stale albums
                   &values);

    QString iconAlbumUrl, iconName;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        AlbumInfo info;

        info.albumRootId    = (*it).toInt();
        ++it;
        info.id             = (*it).toInt();
        ++it;
        info.relativePath   = (*it).toString();
        ++it;
        info.date           = QDate::fromString((*it).toString(), Qt::ISODate);
        ++it;
        info.caption        = (*it).toString();
        ++it;
        info.category       = (*it).toString();
        ++it;
        info.iconId         = (*it).toLongLong();
        ++it;

        aList.append(info);
    }

    return aList;
}

TagInfo::List AlbumDB::scanTags()
{
    TagInfo::List tList;

    QList<QVariant> values;
    d->db->execSql("SELECT id, pid, name, icon, iconkde FROM Tags;", &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        TagInfo info;

        info.id     = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = (*it).toString();
        ++it;
        info.iconId = (*it).toLongLong();
        ++it;
        info.icon   = (*it).toString();
        ++it;

        tList.append(info);
    }

    return tList;
}

TagInfo AlbumDB::getTagInfo(int tagId)
{
    QList<QVariant> values;
    d->db->execSql("SELECT id, pid, name, icon, iconkde WHERE id=? FROM Tags;", tagId, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        TagInfo info;

        info.id     = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = (*it).toString();
        ++it;
        info.iconId = (*it).toLongLong();
        ++it;
        info.icon   = (*it).toString();
        ++it;

        return info;
    }

    return TagInfo();
}

SearchInfo::List AlbumDB::scanSearches()
{
    SearchInfo::List searchList;
    QList<QVariant>  values;

    d->db->execSql("SELECT id, type, name, query FROM Searches;", &values);

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

    d->db->execSql(QString("SELECT Albums.id, Albums.relativePath, Albums.albumRoot from Albums ORDER BY Albums.id; "),
                   &values);

    QList<AlbumShortInfo> albumList;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        AlbumShortInfo info;

        info.id           = (*it).toInt();
        ++it;
        info.relativePath = (*it).toString();
        ++it;
        info.albumRootId  = (*it).toInt();
        ++it;

        albumList << info;
    }

    return albumList;
}

QList<TagShortInfo> AlbumDB::getTagShortInfos()
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT id, pid, name FROM Tags ORDER BY id;"), &values);

    QList<TagShortInfo> tagList;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        TagShortInfo info;

        info.id           = (*it).toInt();
        ++it;
        info.pid          = (*it).toInt();
        ++it;
        info.name         = (*it).toString();
        ++it;

        tagList << info;
    }

    return tagList;
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
    QVariant        id;
    QList<QVariant> boundValues;

    boundValues << albumRootId << relativePath << date.toString(Qt::ISODate) << caption << collection;

    d->db->execSql(QString("REPLACE INTO Albums (albumRoot, relativePath, date, caption, collection) "
                           "VALUES(?, ?, ?, ?, ?);"),
                   boundValues, 0, &id);

    d->db->recordChangeset(AlbumChangeset(id.toInt(), AlbumChangeset::Added));
    return id.toInt();
}

void AlbumDB::setAlbumCaption(int albumID, const QString& caption)
{
    d->db->execSql(QString("UPDATE Albums SET caption=? WHERE id=?;"),
                   caption, albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void AlbumDB::setAlbumCategory(int albumID, const QString& category)
{
    // TODO : change "collection" propertie in DB ALbum table to "category"
    d->db->execSql(QString("UPDATE Albums SET collection=? WHERE id=?;"),
                   category, albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void AlbumDB::setAlbumDate(int albumID, const QDate& date)
{
    d->db->execSql(QString("UPDATE Albums SET date=? WHERE id=?;"),
                   date.toString(Qt::ISODate),
                   albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void AlbumDB::setAlbumIcon(int albumID, qlonglong iconID)
{
    d->db->execSql(QString("UPDATE Albums SET icon=? WHERE id=?;"),
                   iconID,
                   albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void AlbumDB::deleteAlbum(int albumID)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(":albumId", albumID);

    if (DatabaseCoreBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QString("deleteAlbumID")), parameters))
    {
        return;
    }

    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Deleted));
}

void AlbumDB::makeStaleAlbum(int albumID)
{
    // We need to work around the table constraint, no we want to delete older stale albums with
    // the same relativePath, and adjust relativePaths depending on albumRoot.
    QList<QVariant> values;

    // retrieve information
    d->db->execSql(QString("SELECT Albums.albumRoot, Albums.relativePath from Albums WHERE id=?;"),
                   albumID, &values);

    if (values.isEmpty())
    {
        return;
    }

    // prepend albumRootId to relativePath. relativePath is unused and officially undefined after this call.
    QString newRelativePath = values.at(0).toString() + '-' + values.at(1).toString();

    // delete older stale albums
    QMap<QString, QVariant> parameters;
    parameters.insert(":albumRoot", 0);
    parameters.insert(":relativePath", newRelativePath);

    if (DatabaseCoreBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QString("deleteAlbumRootPath")), parameters))
    {
        return;
    }

    // now do our update
    d->db->execSql(QString("UPDATE Albums SET albumRoot=0, relativePath=? WHERE id=?;"),
                   newRelativePath, albumID);

    // for now, we make no distinction to deleteAlbums wrt to changeset
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Deleted));
}

void AlbumDB::deleteStaleAlbums()
{
    QMap<QString, QVariant> parameters;
    parameters.insert(":albumRoot", 0);

    if (DatabaseCoreBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QString("deleteAlbumRoot")), parameters))
    {
        return;
    }

    // deliberately no changeset here, is done above
}

int AlbumDB::addTag(int parentTagID, const QString& name, const QString& iconKDE,
                    qlonglong iconID)
{
    QVariant                id;
    QMap<QString, QVariant> parameters;

    parameters.insert(":tagPID", parentTagID);
    parameters.insert(":tagname", name);

    if (DatabaseCoreBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QString("InsertTag")), parameters, 0 , &id))
    {
        return -1;
    }

    if (!iconKDE.isEmpty())
    {
        d->db->execSql(QString("UPDATE Tags SET iconkde=? WHERE id=?;"),
                       iconKDE,
                       id.toInt());
    }
    else
    {
        d->db->execSql(QString("UPDATE Tags SET icon=? WHERE id=?;"),
                       iconID,
                       id.toInt());
    }

    d->db->recordChangeset(TagChangeset(id.toInt(), TagChangeset::Added));
    return id.toInt();
}

void AlbumDB::deleteTag(int tagID)
{
    /*
    QString("DELETE FROM Tags WHERE id=?;"), tagID
    */

    QMap<QString, QVariant> bindingMap;
    bindingMap.insert(QString(":tagID"), tagID);

    d->db->execDBAction(d->db->getDBAction("DeleteTag"), bindingMap);
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Deleted));
}

void AlbumDB::setTagIcon(int tagID, const QString& iconKDE, qlonglong iconID)
{
    int     _iconID  = iconKDE.isEmpty() ? iconID : 0;
    QString _iconKDE = iconKDE;

    if (iconKDE.isEmpty() || iconKDE.toLower() == QString("tag"))
    {
        _iconKDE.clear();
    }

    d->db->execSql(QString("UPDATE Tags SET iconkde=?, icon=? WHERE id=?;"),
                   _iconKDE, _iconID, tagID);

    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::IconChanged));
}

void AlbumDB::setTagParentID(int tagID, int newParentTagID)
{
    d->db->execSql(QString("UPDATE Tags SET pid=? WHERE id=?;"),
                   newParentTagID, tagID);
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Reparented));
}

QList<TagProperty> AlbumDB::getTagProperties(int tagId)
{
    QList<QVariant> values;

    d->db->execSql("SELECT property, value FROM TagProperties WHERE tagid=?;",
                   tagId, &values);

    QList<TagProperty> properties;

    if (values.isEmpty())
    {
        return properties;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        TagProperty property;

        property.tagId = tagId;

        property.property = (*it).toString();
        ++it;
        property.value    = (*it).toString();
        ++it;

        properties << property;
    }

    return properties;
}

QList<TagProperty> AlbumDB::getTagProperties()
{
    QList<QVariant> values;

    d->db->execSql("SELECT tagid, property, value FROM TagProperties ORDER BY tagid, property;", &values);

    QList<TagProperty> properties;

    if (values.isEmpty())
    {
        return properties;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        TagProperty property;

        property.tagId    = (*it).toInt();
        ++it;
        property.property = (*it).toString();
        ++it;
        property.value    = (*it).toString();
        ++it;

        properties << property;
    }

    return properties;
}

QList< int > AlbumDB::getTagsWithProperty(const QString& property)
{
    QList<QVariant> values;

    d->db->execSql("SELECT DISTINCT tagid FROM TagProperties WHERE property=?;",
                   property, &values);

    QList<int> tagIds;
    foreach(const QVariant & var, values)
    {
        tagIds << var.toInt();
    }
    return tagIds;
}

void AlbumDB::addTagProperty(int tagId, const QString& property, const QString& value)
{
    d->db->execSql("INSERT INTO TagProperties (tagid, property, value) VALUES(?, ?, ?);",
                   tagId, property, value);

    d->db->recordChangeset(TagChangeset(tagId, TagChangeset::PropertiesChanged));
}

void AlbumDB::addTagProperty(const TagProperty& property)
{
    addTagProperty(property.tagId, property.property, property.value);
}

void AlbumDB::removeTagProperties(int tagId, const QString& property, const QString& value)
{
    if (property.isNull())
    {
        d->db->execSql("DELETE FROM TagProperties WHERE tagid=?;",
                       tagId);
    }
    else if (value.isNull())
    {
        d->db->execSql("DELETE FROM TagProperties WHERE tagid=? AND property=?;",
                       tagId, property);
    }
    else
    {
        d->db->execSql("DELETE FROM TagProperties WHERE tagid=? AND property=? AND value=?;",
                       tagId, property, value);
    }

    d->db->recordChangeset(TagChangeset(tagId, TagChangeset::PropertiesChanged));
}

int AlbumDB::addSearch(DatabaseSearch::Type type, const QString& name, const QString& query)
{
    QVariant id;

    if (!d->db->execSql(QString("INSERT INTO Searches (type, name, query) VALUES(?, ?, ?);"),
                        type, name, query, 0, &id))
    {
        return -1;
    }

    d->db->recordChangeset(SearchChangeset(id.toInt(), SearchChangeset::Added));
    return id.toInt();
}

void AlbumDB::updateSearch(int searchID, DatabaseSearch::Type type,
                           const QString& name, const QString& query)
{
    d->db->execSql(QString("UPDATE Searches SET type=?, name=?, query=? WHERE id=?"),
                   type, name, query, searchID);
    d->db->recordChangeset(SearchChangeset(searchID, SearchChangeset::Changed));
}

void AlbumDB::deleteSearch(int searchID)
{
    d->db->execSql(QString("DELETE FROM Searches WHERE id=?"),
                   searchID);
    d->db->recordChangeset(SearchChangeset(searchID, SearchChangeset::Deleted));
}

void AlbumDB::deleteSearches(DatabaseSearch::Type type)
{
    d->db->execSql(QString("DELETE FROM Searches WHERE type=?"),
                   type);
    d->db->recordChangeset(SearchChangeset(0, SearchChangeset::Deleted));
}

QString AlbumDB::getSearchQuery(int searchId)
{
    QList<QVariant> values;
    d->db->execSql(QString("SELECT query FROM Searches WHERE id=?;"),
                   searchId, &values);

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

SearchInfo AlbumDB::getSearchInfo(int searchId)
{
    SearchInfo info;

    QList<QVariant> values;
    d->db->execSql("SELECT id, type, name, query FROM Searches WHERE id=?;",
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
                         const QString& value)
{
    d->db->execSql(QString("REPLACE into Settings VALUES (?,?);"),
                   keyword, value);
}

QString AlbumDB::getSetting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql(QString("SELECT value FROM Settings "
                           "WHERE keyword=?;"),
                   keyword, &values);

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

// helper method
static QStringList joinMainAndUserFilterString(const QString& filter, const QString& userFilter)
{
    QSet<QString> filterSet;
    QStringList   userFilterList;
    QStringList   sortedList;

    filterSet      = filter.split(';', QString::SkipEmptyParts).toSet();
    userFilterList = userFilter.split(';', QString::SkipEmptyParts);

    foreach(const QString& userFormat, userFilterList)
    {
        if (userFormat.startsWith('-'))
        {
            filterSet.remove(userFormat.mid(1));
        }
        else
        {
            filterSet << userFormat;
        }
    }

    sortedList = filterSet.toList();
    sortedList.sort();
    return sortedList;
}

void AlbumDB::getFilterSettings(QStringList* imageFilter, QStringList* videoFilter, QStringList* audioFilter)
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

void AlbumDB::getUserFilterSettings(QString* imageFilterString, QString* videoFilterString, QString* audioFilterString)
{
    if (imageFilterString)
    {
        *imageFilterString = getSetting("databaseUserImageFormats");
    }

    if (videoFilterString)
    {
        *videoFilterString = getSetting("databaseUserVideoFormats");
    }

    if (audioFilterString)
    {
        *audioFilterString = getSetting("databaseUserAudioFormats");
    }
}

void AlbumDB::setFilterSettings(const QStringList& imageFilter, const QStringList& videoFilter, const QStringList& audioFilter)
{
    setSetting("databaseImageFormats", imageFilter.join(";"));
    setSetting("databaseVideoFormats", videoFilter.join(";"));
    setSetting("databaseAudioFormats", audioFilter.join(";"));
}

// helper method
static QStringList cleanUserFilterString(const QString& filterString)
{
    // splits by either ; or space, removes "*.", trims
    QStringList filterList;

    QString wildcard("*.");
    QString minusWildcard("-*.");
    QChar dot('.');
    QString minusDot("-.");
    QChar sep(';');
    int i = filterString.indexOf(sep);

    if (i == -1 && filterString.indexOf(' ') != -1)
    {
        sep = QChar(' ');
    }

    QStringList sepList = filterString.split(sep, QString::SkipEmptyParts);

    foreach(const QString& f, sepList)
    {
        if (f.startsWith(wildcard))
        {
            filterList << f.mid(2).trimmed().toLower();
        }
        else if (f.startsWith(minusWildcard))
        {
            filterList << '-' + f.mid(3).trimmed().toLower();
        }
        else if (f.startsWith(dot))
        {
            filterList << f.mid(1).trimmed().toLower();
        }
        else if (f.startsWith(minusDot))
        {
            filterList << '-' + f.mid(2).trimmed().toLower();
        }
        else
        {
            filterList << f.trimmed().toLower();
        }
    }
    return filterList;
}

void AlbumDB::setUserFilterSettings(const QString& imageFilterString, const QString& videoFilterString,
                                    const QString& audioFilterString)
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
    QStringList addList     = cleanUserFilterString(filterString);
    QStringList currentList = getSetting("databaseUserImageFormats").split(';', QString::SkipEmptyParts);

    // merge lists
    foreach(const QString& addedFilter, addList)
    {
        if (!currentList.contains(addedFilter))
        {
            currentList << addedFilter;
        }
    }

    setSetting("databaseUserImageFormats", currentList.join(";"));
}

QUuid AlbumDB::databaseUuid()
{
    QString uuidString = getSetting("databaseUUID");
    QUuid uuid         = QUuid(uuidString);

    if (uuidString.isNull() || uuid.isNull())
    {
        uuid = QUuid::createUuid();
        setSetting("databaseUUID", uuid.toString());
    }

    return uuid;
}

int AlbumDB::getUniqueHashVersion()
{
    if (d->uniqueHashVersion == -1)
    {
        QString v = getSetting("uniqueHashVersion");

        if (v.isEmpty())
        {
            d->uniqueHashVersion = 1;
        }
        else
        {
            d->uniqueHashVersion = v.toInt();
        }
    }

    return d->uniqueHashVersion;
}

bool AlbumDB::isUniqueHashV2()
{
    return getUniqueHashVersion() == 2;
}

void AlbumDB::setUniqueHashVersion(int version)
{
    d->uniqueHashVersion = version;
    setSetting("uniqueHashVersion", QString::number(d->uniqueHashVersion));
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

    d->db->execSql(QString("SELECT id FROM Images "
                           "WHERE album=? AND name=?;"),
                   albumID,
                   name,
                   &values);

    if (values.isEmpty())
    {
        return -1;
    }
    else
    {
        return values.first().toLongLong();
    }
}

QStringList AlbumDB::getItemTagNames(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT name FROM Tags \n "
                           "WHERE id IN (SELECT tagid FROM ImageTags \n "
                           "             WHERE imageid=?) \n "
                           "ORDER BY name;"),
                   imageID,
                   &values);

    QStringList names;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        names << it->toString();
    }

    return names;
}

QList<int> AlbumDB::getItemTagIDs(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT tagid FROM ImageTags WHERE imageID=?;"),
                   imageID,
                   &values);

    QList<int> ids;

    if (values.isEmpty())
    {
        return ids;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        ids << it->toInt();
    }

    return ids;
}

QVector<QList<int> > AlbumDB::getItemsTagIDs(const QList<qlonglong> imageIds)
{
    if (imageIds.isEmpty())
    {
        return QVector<QList<int> >();
    }

    QVector<QList<int> > results(imageIds.size());
    SqlQuery             query = d->db->prepareQuery("SELECT tagid FROM ImageTags WHERE imageID=?;");
    QVariantList         values;

    for (int i = 0; i < imageIds.size(); i++)
    {
        d->db->execSql(query, imageIds[i], &values);
        QList<int>& tagIds = results[i];

        foreach(const QVariant& v, values)
        {
            tagIds << v.toInt();
        }
    }

    return results;
}

QList<ImageTagProperty> AlbumDB::getImageTagProperties(qlonglong imageId, int tagId)
{
    QList<QVariant> values;

    if (tagId == -1)
    {
        d->db->execSql(QString("SELECT tagid, property, value FROM ImageTagProperties "
                               "WHERE imageid=?;"),
                       imageId,
                       &values);
    }
    else
    {
        d->db->execSql(QString("SELECT tagid, property, value FROM ImageTagProperties "
                               "WHERE imageid=? AND tagid=?;"),
                       imageId, tagId,
                       &values);
    }

    QList<ImageTagProperty> properties;

    if (values.isEmpty())
    {
        return properties;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        ImageTagProperty property;

        property.imageId  = imageId;

        property.tagId    = (*it).toInt();
        ++it;
        property.property = (*it).toString();
        ++it;
        property.value    = (*it).toString();
        ++it;

        properties << property;
    }

    return properties;
}

QList<int> AlbumDB::getTagIdsWithProperties(qlonglong imageId)
{
    QList<QVariant> values;

    d->db->execSql("SELECT DISTINCT tagid FROM ImageTagProperties WHERE imageid=?;",
                   imageId,
                   &values);

    QList<int> tagIds;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        tagIds << (*it).toInt();
    }

    return tagIds;
}

void AlbumDB::addImageTagProperty(qlonglong imageId, int tagId, const QString& property, const QString& value)
{
    d->db->execSql("INSERT INTO ImageTagProperties (imageid, tagid, property, value) VALUES(?, ?, ?, ?);",
                   imageId, tagId, property, value);

    d->db->recordChangeset(ImageTagChangeset(imageId, tagId, ImageTagChangeset::PropertiesChanged));
}

void AlbumDB::addImageTagProperty(const ImageTagProperty& property)
{
    addImageTagProperty(property.imageId, property.tagId, property.property, property.value);
}

void AlbumDB::removeImageTagProperties(qlonglong imageId, int tagId, const QString& property,
                                       const QString& value)
{
    if (tagId == -1)
    {
        d->db->execSql("DELETE FROM ImageTagProperties WHERE imageid=?;",
                       imageId);
    }
    else if (property.isNull())
    {
        d->db->execSql("DELETE FROM ImageTagProperties WHERE imageid=? AND tagid=?;",
                       imageId, tagId);
    }
    else if (value.isNull())
    {
        d->db->execSql("DELETE FROM ImageTagProperties WHERE imageid=? AND tagid=? AND property=?;",
                       imageId, tagId, property);
    }
    else
    {
        d->db->execSql("DELETE FROM ImageTagProperties WHERE imageid=? AND tagid=? AND property=? AND value=?;",
                       imageId, tagId, property, value);
    }

    d->db->recordChangeset(ImageTagChangeset(imageId, tagId, ImageTagChangeset::PropertiesChanged));
}

ItemShortInfo AlbumDB::getItemShortInfo(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT Images.name, Albums.albumRoot, Albums.relativePath, Albums.id "
                           "FROM Images "
                           "  LEFT JOIN Albums ON Albums.id=Images.album "
                           "WHERE Images.id=?;"),
                   imageID,
                   &values);

    ItemShortInfo info;

    if (!values.isEmpty())
    {
        info.id          = imageID;
        info.itemName    = values.at(0).toString();
        info.albumRootID = values.at(1).toInt();
        info.album       = values.at(2).toString();
        info.albumID     = values.at(3).toInt();
    }

    return info;
}

ItemShortInfo AlbumDB::getItemShortInfo(int albumRootId, const QString& relativePath, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT Images.id, Albums.id "
                           " FROM Images INNER JOIN Albums "
                           "  ON Images.album=Albums.id "
                           " WHERE name=? AND albumRoot=? AND relativePath=?;"),
                   name, albumRootId, relativePath,
                   &values);

    ItemShortInfo info;

    if (!values.isEmpty())
    {
        info.id          = values.at(0).toLongLong();
        info.itemName    = name;
        info.albumRootID = albumRootId;
        info.album       = relativePath;
        info.albumID     = values.at(1).toInt();
    }

    return info;
}

bool AlbumDB::hasTags(const QList<qlonglong>& imageIDList)
{
    QList<int> ids;

    if (imageIDList.isEmpty())
    {
        return false;
    }

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
    d->db->execSql(sql, boundValues, &values);

    if (values.isEmpty() || values.first().toInt() == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

QList<int> AlbumDB::getItemCommonTagIDs(const QList<qlonglong>& imageIDList)
{
    QList<int> ids;

    if (imageIDList.isEmpty())
    {
        return ids;
    }

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
    d->db->execSql(sql, boundValues, &values);

    if (values.isEmpty())
    {
        return ids;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        ids << it->toInt();
    }

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
        if ((fields & DatabaseFields::ModificationDate) && !values.isEmpty())
        {
            int index = fieldNames.indexOf("modificationDate");
            values[index] = (values.at(index).isNull() ? QDateTime()
                             : QDateTime::fromString(values.at(index).toString(), Qt::ISODate));
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
        if ((fields & DatabaseFields::CreationDate) && !values.isEmpty())
        {
            int index = fieldNames.indexOf("creationDate");
            values[index] = (values.at(index).isNull() ? QDateTime()
                                                       : QDateTime::fromString(values.at(index).toString(), Qt::ISODate));
        }

        if ((fields & DatabaseFields::DigitizationDate) && !values.isEmpty())
        {
            int index = fieldNames.indexOf("digitizationDate");
            values[index] = (values.at(index).isNull() ? QDateTime()
                                                       : QDateTime::fromString(values.at(index).toString(), Qt::ISODate));
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

        // For some reason, if REAL values may be required from variables stored as QString QVariants. Convert code will come here.
    }

    return values;
}

QVariantList AlbumDB::getVideoMetadata(qlonglong imageID, DatabaseFields::VideoMetadata fields)
{
    QVariantList values;

    if (fields != DatabaseFields::VideoMetadataNone)
    {
        QString query("SELECT ");
        QStringList fieldNames = videoMetadataFieldList(fields);
        query += fieldNames.join(", ");
        query += (" FROM VideoMetadata WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // For some reason REAL values may come as QString QVariants. Convert here.
        if (values.size() == fieldNames.size() &&
            ((fields & DatabaseFields::Aperture) ||
             (fields & DatabaseFields::FocalLength) ||
             (fields & DatabaseFields::FocalLength35) ||
             (fields & DatabaseFields::ExposureTime) ||
             (fields & DatabaseFields::SubjectDistance))
           )
        {
            for (int i = 0; i < values.size(); ++i)
            {
                if (values.at(i).type() == QVariant::String &&
                    (fieldNames.at(i) == "aperture" ||
                     fieldNames.at(i) == "focalLength" ||
                     fieldNames.at(i) == "focalLength35" ||
                     fieldNames.at(i) == "exposureTime" ||
                     fieldNames.at(i) == "subjectDistance")
                   )
                {
                    values[i] = values.at(i).toDouble();
                }
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
        QStringList fieldNames =  imagePositionsFieldList(fields);
        query                  += fieldNames.join(", ");
        query                  += (" FROM ImagePositions WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // For some reason REAL values may come as QString QVariants. Convert here.
        if (values.size() == fieldNames.size() &&
            ((fields & DatabaseFields::LatitudeNumber)      ||
             (fields & DatabaseFields::LongitudeNumber)     ||
             (fields & DatabaseFields::Altitude)            ||
             (fields & DatabaseFields::PositionOrientation) ||
             (fields & DatabaseFields::PositionTilt)        ||
             (fields & DatabaseFields::PositionRoll)        ||
             (fields & DatabaseFields::PositionAccuracy))
           )
        {
            for (int i = 0; i < values.size(); ++i)
            {
                if (values.at(i).type() == QVariant::String &&
                    (fieldNames.at(i) == "latitudeNumber"  ||
                     fieldNames.at(i) == "longitudeNumber" ||
                     fieldNames.at(i) == "altitude"        ||
                     fieldNames.at(i) == "orientation"     ||
                     fieldNames.at(i) == "tilt"            ||
                     fieldNames.at(i) == "roll"            ||
                     fieldNames.at(i) == "accuracy")
                   )
                {
                    values[i] = values.at(i).toDouble();
                }
            }
        }
    }

    return values;
}

QVariantList AlbumDB::getImagePositions(QList<qlonglong> imageIDs, DatabaseFields::ImagePositions fields)
{
    QVariantList values;

    if (fields != DatabaseFields::ImagePositionsNone)
    {
        QString sql("SELECT ");
        QStringList fieldNames =  imagePositionsFieldList(fields);
        sql                    += fieldNames.join(", ");
        sql                    += (" FROM ImagePositions WHERE imageid=?;");

        SqlQuery query = d->db->prepareQuery(sql);

        foreach(const qlonglong& imageid, imageIDs)
        {
            QVariantList singleValueList;
            d->db->execSql(query, imageid, &singleValueList);
            values << singleValueList;
        }

        // For some reason REAL values may come as QString QVariants. Convert here.
        if (values.size() == fieldNames.size() &&
            (fields & DatabaseFields::LatitudeNumber      ||
             fields & DatabaseFields::LongitudeNumber     ||
             fields & DatabaseFields::Altitude            ||
             fields & DatabaseFields::PositionOrientation ||
             fields & DatabaseFields::PositionTilt        ||
             fields & DatabaseFields::PositionRoll        ||
             fields & DatabaseFields::PositionAccuracy)
           )
        {
            for (int i = 0; i < values.size(); ++i)
            {
                if (values.at(i).type() == QVariant::String &&
                    (fieldNames.at(i) == "latitudeNumber"  ||
                     fieldNames.at(i) == "longitudeNumber" ||
                     fieldNames.at(i) == "altitude"        ||
                     fieldNames.at(i) == "orientation"     ||
                     fieldNames.at(i) == "tilt"            ||
                     fieldNames.at(i) == "roll"            ||
                     fieldNames.at(i) == "accuracy")
                   )
                {
                    values[i] = values.at(i).toDouble();
                }
            }
        }
    }

    return values;
}

void AlbumDB::addImageInformation(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageInformation fields)
{
    if (fields == DatabaseFields::ImageInformationNone)
    {
        return;
    }

    QString query("REPLACE INTO ImageInformation ( imageid, ");

    QStringList fieldNames = imageInformationFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(", ");
    query += " ) VALUES (";
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += ");";

    QVariantList boundValues;
    boundValues << imageID;

    // Take care for datetime values
    if ((fields & DatabaseFields::CreationDate) || (fields & DatabaseFields::DigitizationDate))
    {
        foreach(const QVariant& value, infos)
        {
            if (value.type() == QVariant::DateTime || value.type() == QVariant::Date)
            {
                boundValues << value.toDateTime().toString(Qt::ISODate);
            }
            else
            {
                boundValues << value;
            }
        }
    }
    else
    {
        boundValues << infos;
    }

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::changeImageInformation(qlonglong imageId, const QVariantList& infos,
                                     DatabaseFields::ImageInformation fields)
{
    if (fields == DatabaseFields::ImageInformationNone)
    {
        return;
    }

    QStringList fieldNames = imageInformationFieldList(fields);

    QVariantList checkedValues = infos;

    // Convert dateTime values to the appropriate string format
    if (fields & DatabaseFields::CreationDate || fields & DatabaseFields::DigitizationDate)
    {
        for (QVariantList::iterator it = checkedValues.begin(); it != checkedValues.end(); ++it)
        {
            if (it->type() == QVariant::DateTime || it->type() == QVariant::Date)
            {
                *it = it->toDateTime().toString(Qt::ISODate);
            }
        }
    }

    d->db->execUpsertDBAction("changeImageInformation", imageId, fieldNames, checkedValues);

    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void AlbumDB::addImageMetadata(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageMetadata fields)
{
    if (fields == DatabaseFields::ImageMetadataNone)
    {
        return;
    }

    QString query("REPLACE INTO ImageMetadata ( imageid, ");
    QStringList fieldNames = imageMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(", ");
    query += " ) VALUES (";
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += ");";

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::changeImageMetadata(qlonglong imageId, const QVariantList& infos,
                                  DatabaseFields::ImageMetadata fields)
{
    if (fields == DatabaseFields::ImageMetadataNone)
    {
        return;
    }

    QString query("UPDATE ImageMetadata SET ");

    QStringList fieldNames = imageMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join("=?,");
    query += "=? WHERE imageid=?;";

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void AlbumDB::addVideoMetadata(qlonglong imageID, const QVariantList& infos, DatabaseFields::VideoMetadata fields)
{
    if (fields == DatabaseFields::VideoMetadataNone)
    {
        return;
    }

    QString query("REPLACE INTO VideoMetadata ( imageid, "); //need to create this database
    QStringList fieldNames = videoMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(", ");
    query += " ) VALUES (";
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += ");";

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::changeVideoMetadata(qlonglong imageId, const QVariantList& infos,
                                  DatabaseFields::VideoMetadata fields)
{
    if (fields == DatabaseFields::VideoMetadataNone)
    {
        return;
    }

    QString query("UPDATE VideoMetadata SET ");
    QStringList fieldNames = videoMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join("=?,");
    query += "=? WHERE imageid=?;";

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void AlbumDB::addImagePosition(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImagePositions fields)
{
    if (fields == DatabaseFields::ImagePositionsNone)
    {
        return;
    }

    QString query("REPLACE INTO ImagePositions ( imageid, ");
    QStringList fieldNames = imagePositionsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(", ");
    query += " ) VALUES (";
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += ");";

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::changeImagePosition(qlonglong imageId, const QVariantList& infos,
                                  DatabaseFields::ImagePositions fields)
{
    if (fields == DatabaseFields::ImagePositionsNone)
    {
        return;
    }

    QString query("UPDATE ImagePositions SET ");
    QStringList fieldNames = imagePositionsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join("=?,");
    query += "=? WHERE imageid=?;";

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void AlbumDB::removeImagePosition(qlonglong imageid)
{
    d->db->execSql(QString("DELETE FROM ImagePositions WHERE imageid=?;"),
                   imageid);

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::ImagePositionsAll));
}

QList<CommentInfo> AlbumDB::getImageComments(qlonglong imageID)
{
    QList<CommentInfo> list;

    QList<QVariant> values;
    d->db->execSql(QString("SELECT id, type, language, author, date, comment "
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
    d->db->execSql(QString("REPLACE INTO ImageComments "
                           "( imageid, type, language, author, date, comment ) "
                           " VALUES (?,?,?,?,?,?);"),
                   boundValues, 0, &id);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::ImageCommentsAll));
    return id.toInt();
}

void AlbumDB::changeImageComment(int commentId, qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageComments fields)
{
    if (fields == DatabaseFields::ImageCommentsNone)
    {
        return;
    }

    QString query("UPDATE ImageComments SET ");
    QStringList fieldNames = imageCommentsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join("=?,");
    query += "=? WHERE id=?;";

    QVariantList boundValues;
    boundValues << infos << commentId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void AlbumDB::removeImageComment(int commentid, qlonglong imageid)
{
    d->db->execSql(QString("DELETE FROM ImageComments WHERE id=?;"),
                   commentid);

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::ImageCommentsAll));
}

QString AlbumDB::getImageProperty(qlonglong imageID, const QString& property)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT value FROM ImageProperties "
                           "WHERE imageid=? and property=?;"),
                   imageID, property,
                   &values);

    if (!values.isEmpty())
    {
        return values.first().toString();
    }
    else
    {
        return QString();
    }
}

void AlbumDB::setImageProperty(qlonglong imageID, const QString& property, const QString& value)
{
    d->db->execSql(QString("REPLACE INTO ImageProperties "
                           "(imageid, property, value) "
                           "VALUES(?, ?, ?);"),
                   imageID, property, value);
}

void AlbumDB::removeImageProperty(qlonglong imageID, const QString& property)
{
    d->db->execSql(QString("DELETE FROM ImageProperties WHERE imageid=? AND property=?;"),
                   imageID, property);
}

QList<CopyrightInfo> AlbumDB::getImageCopyright(qlonglong imageID, const QString& property)
{
    QList<CopyrightInfo> list;
    QList<QVariant>      values;

    if (property.isNull())
    {
        d->db->execSql(QString("SELECT property, value, extraValue FROM ImageCopyright "
                               "WHERE imageid=?;"),
                       imageID, &values);
    }
    else
    {
        d->db->execSql(QString("SELECT property, value, extraValue FROM ImageCopyright "
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
        d->db->execSql(QString("DELETE FROM ImageCopyright "
                               "WHERE imageid=? AND property=?;"),
                       imageID, property);
    }
    else if (uniqueness == PropertyExtraValueUnique)
    {
        d->db->execSql(QString("DELETE FROM ImageCopyright "
                               "WHERE imageid=? AND property=? AND extraValue=?;"),
                       imageID, property, extraValue);
    }

    d->db->execSql(QString("REPLACE INTO ImageCopyright "
                           "(imageid, property, value, extraValue) "
                           "VALUES(?, ?, ?, ?);"),
                   imageID, property, value, extraValue);
}

void AlbumDB::removeImageCopyrightProperties(qlonglong imageID, const QString& property,
                                             const QString& extraValue, const QString& value)
{
    int removeBy = 0;

    if (!property.isNull())
    {
        ++removeBy;
    }
    if (!extraValue.isNull())
    {
        ++removeBy;
    }
    if (!value.isNull())
    {
        ++removeBy;
    }

    switch (removeBy)
    {
        case 0:
            d->db->execSql(QString("DELETE FROM ImageCopyright "
                                   "WHERE imageid=?;"),
                           imageID);
            break;

        case 1:
            d->db->execSql(QString("DELETE FROM ImageCopyright "
                                   "WHERE imageid=? AND property=?;"),
                           imageID, property);
            break;

        case 2:
            d->db->execSql(QString("DELETE FROM ImageCopyright "
                                   "WHERE imageid=? AND property=? AND extraValue=?;"),
                           imageID, property, extraValue);
            break;

        case 3:
            d->db->execSql(QString("DELETE FROM ImageCopyright "
                                   "WHERE imageid=? AND property=? AND extraValue=? AND value=?;"),
                           imageID, property, extraValue, value);
            break;
    }
}

QList<qlonglong> AlbumDB::findByNameAndCreationDate(const QString& fileName, const QDateTime& creationDate)
{
    QList<QVariant> values;

    d->db->execSql("SELECT id FROM Images "
                   " INNER JOIN ImageInformation ON id=imageid "
                   "WHERE name=? AND creationDate=? AND status!=3;",
                   fileName, creationDate.toString(Qt::ISODate), &values);

    QList<qlonglong> ids;
    foreach(const QVariant& var, values)
    {
        ids << var.toLongLong();
    }
    return ids;
}

bool AlbumDB::hasImageHistory(qlonglong imageId)
{
    QList<QVariant> values;

    d->db->execSql("SELECT history FROM ImageHistory WHERE imageid=?;",
                   imageId, &values);

    return !values.isEmpty();
}

ImageHistoryEntry AlbumDB::getImageHistory(qlonglong imageId)
{
    QList<QVariant> values;

    d->db->execSql("SELECT uuid, history FROM ImageHistory WHERE imageid=?;",
                   imageId, &values);

    ImageHistoryEntry entry;
    entry.imageId = imageId;

    if (values.count() != 2)
    {
        return entry;
    }

    QList<QVariant>::const_iterator it = values.constBegin();

    entry.uuid    = (*it).toString();
    ++it;
    entry.history = (*it).toString();
    ++it;

    return entry;
}

QList<qlonglong> AlbumDB::getItemsForUuid(const QString& uuid)
{
    QList<QVariant> values;

    d->db->execSql("SELECT imageid FROM ImageHistory "
                   "INNER JOIN Images ON imageid=id "
                   "WHERE uuid=? AND status!=3;",
                   uuid, &values);

    QList<qlonglong> imageIds;

    if (values.isEmpty())
    {
        return imageIds;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        imageIds << (*it).toInt();
    }

    return imageIds;
}

QString AlbumDB::getImageUuid(qlonglong imageId)
{
    QList<QVariant> values;

    d->db->execSql("SELECT uuid FROM ImageHistory WHERE imageid=?;",
                   imageId, &values);

    if (values.isEmpty())
    {
        return QString();
    }

    QString uuid = values.first().toString();

    if (uuid.isEmpty())
    {
        return QString();
    }

    return uuid;
}

void AlbumDB::setImageHistory(qlonglong imageId, const QString& history)
{
    d->db->execUpsertDBAction("changeImageHistory", imageId, QStringList() << "history", QVariantList() << history);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::ImageHistory));
}

void AlbumDB::setImageUuid(qlonglong imageId, const QString& uuid)
{
    d->db->execUpsertDBAction("changeImageHistory", imageId, QStringList() << "uuid", QVariantList() << uuid);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::ImageUUID));
}

void AlbumDB::addImageRelation(qlonglong subjectId, qlonglong objectId, DatabaseRelation::Type type)
{
    d->db->execSql("REPLACE INTO ImageRelations (subject, object, type) VALUES (?, ?, ?);",
                   subjectId, objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << subjectId << objectId, DatabaseFields::ImageRelations));
}

void AlbumDB::addImageRelations(const QList<qlonglong>& subjectIds, const QList<qlonglong>& objectIds, DatabaseRelation::Type type)
{
    SqlQuery query = d->db->prepareQuery("REPLACE INTO ImageRelations (subject, object, type) VALUES (?, ?, ?);");

    QVariantList subjects, objects, types;

    for (int i = 0; i < subjectIds.size(); ++i)
    {
        subjects << subjectIds.at(i);
        objects  << objectIds.at(i);
        types    << type;
    }

    query.addBindValue(subjects);
    query.addBindValue(objects);
    query.addBindValue(types);
    d->db->execBatch(query);
    d->db->recordChangeset(ImageChangeset(subjectIds + objectIds, DatabaseFields::ImageRelations));
}


void AlbumDB::addImageRelation(const ImageRelation& relation)
{
    addImageRelation(relation.subjectId, relation.objectId, relation.type);
}

void AlbumDB::removeImageRelation(qlonglong subjectId, qlonglong objectId, DatabaseRelation::Type type)
{
    d->db->execSql("DELETE FROM ImageRelations WHERE subject=? AND object=? AND type=?;",
                   subjectId, objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << subjectId << objectId, DatabaseFields::ImageRelations));
}

void AlbumDB::removeImageRelation(const ImageRelation& relation)
{
    removeImageRelation(relation.subjectId, relation.objectId, relation.type);
}

QList<qlonglong> AlbumDB::removeAllImageRelationsTo(qlonglong objectId, DatabaseRelation::Type type)
{
    QList<qlonglong> affected = getImagesRelatingTo(objectId, type);

    if (affected.isEmpty())
    {
        return affected;
    }

    d->db->execSql("DELETE FROM ImageRelations WHERE object=? AND type=?;",
                   objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << affected << objectId, DatabaseFields::ImageRelations));

    return affected;
}

QList<qlonglong> AlbumDB::removeAllImageRelationsFrom(qlonglong subjectId, DatabaseRelation::Type type)
{
    QList<qlonglong> affected = getImagesRelatedFrom(subjectId, type);

    if (affected.isEmpty())
    {
        return affected;
    }

    d->db->execSql("DELETE FROM ImageRelations WHERE subject=? AND type=?;",
                   subjectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << affected << subjectId, DatabaseFields::ImageRelations));

    return affected;
}

QList<qlonglong> AlbumDB::getImagesRelatedFrom(qlonglong subjectId, DatabaseRelation::Type type)
{
    return getRelatedImages(subjectId, true, type, false);
}

QVector<QList<qlonglong> > AlbumDB::getImagesRelatedFrom(QList<qlonglong> subjectIds, DatabaseRelation::Type type)
{
    return getRelatedImages(subjectIds, true, type, false);
}

bool AlbumDB::hasImagesRelatedFrom(qlonglong subjectId, DatabaseRelation::Type type)
{
    // returns 0 or 1 item in list
    return !getRelatedImages(subjectId, true, type, true).isEmpty();
}

QList<qlonglong> AlbumDB::getImagesRelatingTo(qlonglong objectId, DatabaseRelation::Type type)
{
    return getRelatedImages(objectId, false, type, false);
}

QVector<QList<qlonglong> > AlbumDB::getImagesRelatingTo(QList<qlonglong> objectIds, DatabaseRelation::Type type)
{
    return getRelatedImages(objectIds, false, type, false);
}

bool AlbumDB::hasImagesRelatingTo(qlonglong objectId, DatabaseRelation::Type type)
{
    // returns 0 or 1 item in list
    return !getRelatedImages(objectId, false, type, true).isEmpty();
}

QList<qlonglong> AlbumDB::getRelatedImages(qlonglong id, bool fromOrTo, DatabaseRelation::Type type, bool boolean)
{
    QString sql = d->constructRelatedImagesSQL(fromOrTo, type, boolean);
    SqlQuery query = d->db->prepareQuery(sql);
    return d->execRelatedImagesQuery(query, id, type);
}

QVector<QList<qlonglong> > AlbumDB::getRelatedImages(QList<qlonglong> ids,
                                                     bool fromOrTo, DatabaseRelation::Type type, bool boolean)
{
    if (ids.isEmpty())
    {
        return QVector<QList<qlonglong> >();
    }

    QVector<QList<qlonglong> > result(ids.size());

    QString sql = d->constructRelatedImagesSQL(fromOrTo, type, boolean);
    SqlQuery query = d->db->prepareQuery(sql);

    for (int i = 0; i < ids.size(); i++)
    {
        result[i] = d->execRelatedImagesQuery(query, ids[i], type);
    }

    return result;
}

QList<QPair<qlonglong, qlonglong> > AlbumDB::getRelationCloud(qlonglong imageId, DatabaseRelation::Type type)
{
    QSet<qlonglong> todo, done;
    QSet<QPair<qlonglong, qlonglong> > pairs;
    todo << imageId;

    QString sql = "SELECT subject, object FROM ImageRelations "
                  "INNER JOIN Images AS SubjectImages ON ImageRelations.subject=SubjectImages.id "
                  "INNER JOIN Images AS ObjectImages  ON ImageRelations.object=ObjectImages.id "
                  "WHERE (subject=? OR object=?) %1 AND SubjectImages.status!=3 AND ObjectImages.status!=3;";

    if (type == DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString());
    }
    else
    {
        sql = sql.arg("AND type=?");
    }

    SqlQuery query = d->db->prepareQuery(sql);

    QList<QVariant> values;
    qlonglong subject, object;

    while (!todo.isEmpty())
    {
        qlonglong id = *todo.begin();
        todo.erase(todo.begin());
        done << id;

        if (type == DatabaseRelation::UndefinedType)
        {
            d->db->execSql(query, id, id, &values);
        }
        else
        {
            d->db->execSql(query, id, id, type, &values);
        }

        for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
        {
            subject = (*it).toLongLong();
            ++it;
            object = (*it).toLongLong();
            ++it;

            pairs << qMakePair(subject, object);

            if (!done.contains(subject))
            {
                todo << subject;
            }

            if (!done.contains(object))
            {
                todo << object;
            }
        }
    }

    return pairs.toList();
}

QList<qlonglong> AlbumDB::getOneRelatedImageEach(const QList<qlonglong>& ids, DatabaseRelation::Type type)
{
    QString sql = "SELECT subject, object FROM ImageRelations "
                  "INNER JOIN Images AS SubjectImages ON ImageRelations.subject=SubjectImages.id "
                  "INNER JOIN Images AS ObjectImages  ON ImageRelations.object=ObjectImages.id "
                  "WHERE ( (subject=? AND ObjectImages.status!=3) "
                  "     OR (object=? AND SubjectImages.status!=3) ) "
                  " %1 LIMIT 1;";

    if (type == DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString());
    }
    else
    {
        sql = sql.arg("AND type=?");
    }

    SqlQuery        query = d->db->prepareQuery(sql);
    QSet<qlonglong> result;
    QList<QVariant> values;

    foreach(qlonglong id, ids)
    {
        if (type == DatabaseRelation::UndefinedType)
        {
            d->db->execSql(query, id, id, &values);
        }
        else
        {
            d->db->execSql(query, id, id, type, &values);
        }

        if (values.size() != 2)
        {
            continue;
        }

        // one of subject and object is the given id, the other our result
        if (values.first() != id)
        {
            result << values.first().toLongLong();
        }
        else
        {
            result << values.last().toLongLong();
        }
    }

    return result.toList();
}

bool AlbumDB::hasHaarFingerprints() const
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT imageid FROM ImageHaarMatrix "
                           "WHERE matrix IS NOT NULL LIMIT 1;"),
                   &values);

    // return true if there is at least one fingerprint
    return !values.isEmpty();
}

QList<qlonglong> AlbumDB::getDirtyOrMissingFingerprints()
{
    QList<qlonglong> itemIDs;
    QList<QVariant>  values;

    d->db->execSql(QString("SELECT id FROM Images "
                           "LEFT JOIN ImageHaarMatrix ON Images.id=ImageHaarMatrix.imageid "
                           " WHERE Images.status=1 AND Images.category=1 AND "
                           " ( ImageHaarMatrix.imageid IS NULL "
                           "   OR Images.modificationDate != ImageHaarMatrix.modificationDate "
                           "   OR Images.uniqueHash != ImageHaarMatrix.uniqueHash ); "),
                   &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

QStringList AlbumDB::getDirtyOrMissingFingerprintURLs()
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
                           "LEFT JOIN ImageHaarMatrix ON Images.id=ImageHaarMatrix.imageid "
                           "LEFT JOIN Albums ON Albums.id=Images.album "
                           " WHERE Images.status=1 AND Images.category=1 AND "
                           " ( ImageHaarMatrix.imageid IS NULL "
                           "   OR Images.modificationDate != ImageHaarMatrix.modificationDate "
                           "   OR Images.uniqueHash != ImageHaarMatrix.uniqueHash ); "),
                   &values);

    QStringList urls;
    QString     albumRootPath, relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;

        if (relativePath == "/")
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + '/' + name;
        }
    }

    return urls;
}

QStringList AlbumDB::getItemsURLsWithTag(int tagId)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
                           "LEFT JOIN ImageTags ON Images.id=ImageTags.imageid "
                           "LEFT JOIN Albums ON Albums.id=Images.album "
                           " WHERE Images.status=1 AND Images.category=1 AND ImageTags.tagid=?; "),
                   tagId, &values);
    
    QStringList urls;
    QString     albumRootPath, relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;

        if (relativePath == "/")
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + '/' + name;
        }
    }

    return urls;
}

QStringList AlbumDB::getDirtyOrMissingFaceImageUrls()
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
                           "LEFT JOIN ImageScannedMatrix ON Images.id=ImageScannedMatrix.imageid "
                           "LEFT JOIN Albums ON Albums.id=Images.album "
                           " WHERE Images.status=1 AND Images.category=1 AND "
                           " ( ImageScannedMatrix.imageid IS NULL "
                           "   OR Images.modificationDate != ImageScannedMatrix.modificationDate "
                           "   OR Images.uniqueHash != ImageScannedMatrix.uniqueHash ); "),
                   &values);

    QStringList urls;
    QString     albumRootPath, relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;

        if (relativePath == "/")
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + '/' + name;
        }
    }

    return urls;
}

QList<ItemScanInfo> AlbumDB::getIdenticalFiles(qlonglong id)
{
    if (!id)
    {
        return QList<ItemScanInfo>();
    }

    QList<QVariant> values;

    // retrieve unique hash and file size
    d->db->execSql(QString("SELECT uniqueHash, fileSize FROM Images WHERE id=?; "),
                   id,
                   &values);

    if (values.isEmpty())
    {
        return QList<ItemScanInfo>();
    }

    QString uniqueHash = values.at(0).toString();
    qlonglong fileSize = values.at(1).toLongLong();

    return getIdenticalFiles(uniqueHash, fileSize, id);
}

QList<ItemScanInfo> AlbumDB::getIdenticalFiles(const QString& uniqueHash, qlonglong fileSize, qlonglong sourceId)
{
    // enforce validity
    if (uniqueHash.isEmpty() || fileSize <= 0)
    {
        return QList<ItemScanInfo>();
    }

    QList<QVariant> values;

    // find items with same fingerprint
    d->db->execSql(QString("SELECT id, album, name, status, category, modificationDate, fileSize FROM Images "
                           " WHERE fileSize=? AND uniqueHash=?; "),
                   fileSize, uniqueHash,
                   &values);

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
                                 : QDateTime::fromString((*it).toString(), Qt::ISODate));
        ++it;
        info.fileSize         = (*it).toLongLong();
        ++it;

        // exclude one source id from list
        if (sourceId == info.id)
        {
            continue;
        }

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
    {
        list << "album";
    }

    if (fields & DatabaseFields::Name)
    {
        list << "name";
    }

    if (fields & DatabaseFields::Status)
    {
        list << "status";
    }

    if (fields & DatabaseFields::Category)
    {
        list << "category";
    }

    if (fields & DatabaseFields::ModificationDate)
    {
        list << "modificationDate";
    }

    if (fields & DatabaseFields::FileSize)
    {
        list << "fileSize";
    }

    if (fields & DatabaseFields::UniqueHash)
    {
        list << "uniqueHash";
    }

    return list;
}

QStringList AlbumDB::imageInformationFieldList(DatabaseFields::ImageInformation fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::Rating)
    {
        list << "rating";
    }

    if (fields & DatabaseFields::CreationDate)
    {
        list << "creationDate";
    }

    if (fields & DatabaseFields::DigitizationDate)
    {
        list << "digitizationDate";
    }

    if (fields & DatabaseFields::Orientation)
    {
        list << "orientation";
    }

    if (fields & DatabaseFields::Width)
    {
        list << "width";
    }

    if (fields & DatabaseFields::Height)
    {
        list << "height";
    }

    if (fields & DatabaseFields::Format)
    {
        list << "format";
    }

    if (fields & DatabaseFields::ColorDepth)
    {
        list << "colorDepth";
    }

    if (fields & DatabaseFields::ColorModel)
    {
        list << "colorModel";
    }

    return list;
}

QStringList AlbumDB::videoMetadataFieldList(DatabaseFields::VideoMetadata fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::AspectRatio)
    {
        list << "aspectRatio";
    }

    if (fields & DatabaseFields::AudioBitRate)
    {
        list << "audioBitRate";
    }

    if (fields & DatabaseFields::AudioChannelType)
    {
        list << "audioChannelType";
    }

    if (fields & DatabaseFields::AudioCompressor)
    {
        list << "audioCompressor";
    }

    if (fields & DatabaseFields::Duration)
    {
        list << "duration";
    }

    if (fields & DatabaseFields::FrameRate)
    {
        list << "frameRate";
    }

    if (fields & DatabaseFields::VideoCodec)
    {
        list << "videoCodec";
    }

    return list;
}

QStringList AlbumDB::imageMetadataFieldList(DatabaseFields::ImageMetadata fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::Make)
    {
        list << "make";
    }

    if (fields & DatabaseFields::Model)
    {
        list << "model";
    }

    if (fields & DatabaseFields::Lens)
    {
        list << "lens";
    }

    if (fields & DatabaseFields::Aperture)
    {
        list << "aperture";
    }

    if (fields & DatabaseFields::FocalLength)
    {
        list << "focalLength";
    }

    if (fields & DatabaseFields::FocalLength35)
    {
        list << "focalLength35";
    }

    if (fields & DatabaseFields::ExposureTime)
    {
        list << "exposureTime";
    }

    if (fields & DatabaseFields::ExposureProgram)
    {
        list << "exposureProgram";
    }

    if (fields & DatabaseFields::ExposureMode)
    {
        list << "exposureMode";
    }

    if (fields & DatabaseFields::Sensitivity)
    {
        list << "sensitivity";
    }

    if (fields & DatabaseFields::FlashMode)
    {
        list << "flash";
    }

    if (fields & DatabaseFields::WhiteBalance)
    {
        list << "whiteBalance";
    }

    if (fields & DatabaseFields::WhiteBalanceColorTemperature)
    {
        list << "whiteBalanceColorTemperature";
    }

    if (fields & DatabaseFields::MeteringMode)
    {
        list << "meteringMode";
    }

    if (fields & DatabaseFields::SubjectDistance)
    {
        list << "subjectDistance";
    }

    if (fields & DatabaseFields::SubjectDistanceCategory)
    {
        list << "subjectDistanceCategory";
    }

    return list;
}

QStringList AlbumDB::imagePositionsFieldList(DatabaseFields::ImagePositions fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::Latitude)
    {
        list << "latitude";
    }

    if (fields & DatabaseFields::LatitudeNumber)
    {
        list << "latitudeNumber";
    }

    if (fields & DatabaseFields::Longitude)
    {
        list << "longitude";
    }

    if (fields & DatabaseFields::LongitudeNumber)
    {
        list << "longitudeNumber";
    }

    if (fields & DatabaseFields::Altitude)
    {
        list << "altitude";
    }

    if (fields & DatabaseFields::PositionOrientation)
    {
        list << "orientation";
    }

    if (fields & DatabaseFields::PositionTilt)
    {
        list << "tilt";
    }

    if (fields & DatabaseFields::PositionRoll)
    {
        list << "roll";
    }

    if (fields & DatabaseFields::PositionAccuracy)
    {
        list << "accuracy";
    }

    if (fields & DatabaseFields::PositionDescription)
    {
        list << "description";
    }

    return list;
}

QStringList AlbumDB::imageCommentsFieldList(DatabaseFields::ImageComments fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::CommentType)
    {
        list << "type";
    }

    if (fields & DatabaseFields::CommentLanguage)
    {
        list << "language";
    }

    if (fields & DatabaseFields::CommentAuthor)
    {
        list << "author";
    }

    if (fields & DatabaseFields::CommentDate)
    {
        list << "date";
    }

    if (fields & DatabaseFields::Comment)
    {
        list << "comment";
    }

    return list;
}

void AlbumDB::addBoundValuePlaceholders(QString& query, int count)
{
    // adds no spaces at beginning or end
    QString questionMarks;
    questionMarks.reserve(count * 2);
    QString questionMark("?,");

    for (int i = 0; i < count; ++i)
    {
        questionMarks += questionMark;
    }

    // remove last ','
    questionMarks.chop(1);

    query += questionMarks;
}

int AlbumDB::findInDownloadHistory(const QString& identifier, const QString& name, qlonglong fileSize, const QDateTime& date)
{
    QList<QVariant> values;
    d->db->execSql(QString("SELECT id FROM DownloadHistory WHERE "
                           "identifier=? AND filename=? AND filesize=? AND filedate=?;"),
                   identifier, name, fileSize, date.toString(Qt::ISODate), &values);

    if (values.isEmpty())
    {
        return -1;
    }

    return values.first().toInt();
}

int AlbumDB::addToDownloadHistory(const QString& identifier, const QString& name, qlonglong fileSize, const QDateTime& date)
{
    QVariant id;
    d->db->execSql(QString("REPLACE INTO DownloadHistory "
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
    d->db->execSql(QString("REPLACE INTO ImageTags (imageid, tagid) "
                           "VALUES(?, ?);"),
                   imageID,
                   tagID);

    d->db->recordChangeset(ImageTagChangeset(imageID, tagID, ImageTagChangeset::Added));

    TagsCache * tc = TagsCache::instance();

    //don't save pick or color tags
    if (tc->isInternalTag(tagID))
        return;

    //move current tag to front
    d->recentlyAssignedTags.removeAll(tagID);
    d->recentlyAssignedTags.push_front(tagID);
    if (d->recentlyAssignedTags.size() > 10)
    {
        d->recentlyAssignedTags.pop_back();
    }

    writeSettings();
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
    if (imageIDs.isEmpty() || tagIDs.isEmpty())
    {
        return;
    }

    SqlQuery     query = d->db->prepareQuery("REPLACE INTO ImageTags (imageid, tagid) VALUES(?, ?);");
    QVariantList images;
    QVariantList tags;

    foreach(const qlonglong& imageid, imageIDs)
    {
        foreach(int tagid, tagIDs)
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
    d->db->execSql(QString("DELETE FROM ImageTags "
                           "WHERE imageID=? AND tagid=?;"),
                   imageID,
                   tagID);

    d->db->recordChangeset(ImageTagChangeset(imageID, tagID, ImageTagChangeset::Removed));
}

void AlbumDB::removeItemAllTags(qlonglong imageID, const QList<int>& currentTagIds)
{
    d->db->execSql(QString("DELETE FROM ImageTags "
                           "WHERE imageID=?;"),
                   imageID);

    d->db->recordChangeset(ImageTagChangeset(imageID, currentTagIds, ImageTagChangeset::RemovedAll));
}

void AlbumDB::removeTagsFromItems(QList<qlonglong> imageIDs, const QList<int>& tagIDs)
{
    if (imageIDs.isEmpty() || tagIDs.isEmpty())
    {
        return;
    }

    SqlQuery     query = d->db->prepareQuery("DELETE FROM ImageTags WHERE imageID=? AND tagid=?;");
    QVariantList images;
    QVariantList tags;

    foreach(const qlonglong& imageid, imageIDs)
    {
        foreach(int tagid, tagIDs)
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

QStringList AlbumDB::getItemNamesInAlbum(int albumID, bool recursive)
{
    QList<QVariant> values;

    if (recursive)
    {
        int rootId = getAlbumRootId(albumID);
        QString path = getAlbumRelativePath(albumID);
        d->db->execSql(QString("SELECT Images.name FROM Images WHERE Images.album IN "
                               " (SELECT DISTINCT id FROM Albums "
                               "  WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?));"),
                       rootId, path, path == "/" ? "/%" : QString(path + QLatin1String("/%")),
                       &values);
    }
    else
    {
        d->db->execSql(QString("SELECT Images.name "
                               "FROM Images "
                               "WHERE Images.album=?"),
                       albumID, &values);
    }

    QStringList names;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        names << it->toString();
    }

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
    d->db->execSql("SELECT creationDate FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.status=1;", &values);

    QList<QDateTime> list;

    foreach(const QVariant& value, values)
    {
        if (!value.isNull())
        {
            list << QDateTime::fromString(value.toString(), Qt::ISODate);
        }
    }
    return list;
}

QMap<QDateTime, int> AlbumDB::getAllCreationDatesAndNumberOfImages()
{
    QList<QVariant> values;
    d->db->execSql("SELECT creationDate FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.status=1;", &values);

    QMap<QDateTime, int> datesStatMap;

    foreach(const QVariant& value, values)
    {
        if (!value.isNull())
        {
            QDateTime dateTime = QDateTime::fromString(value.toString(), Qt::ISODate);

            if (!dateTime.isValid())
            {
                continue;
            }

            QMap<QDateTime, int>::iterator it2 = datesStatMap.find(dateTime);

            if (it2 == datesStatMap.end())
            {
                datesStatMap.insert(dateTime, 1);
            }
            else
            {
                it2.value()++;
            }
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

    d->db->execSql("SELECT album FROM Images WHERE Images.status=1;", &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumID = (*it).toInt();
        ++it;

        QMap<int, int>::iterator it2 = albumsStatMap.find(albumID);

        if (it2 == albumsStatMap.end())
        {
            albumsStatMap.insert(albumID, 1);
        }
        else
        {
            it2.value()++;
        }
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

    d->db->execSql("SELECT tagid FROM ImageTags "
                   " LEFT JOIN Images ON Images.id=ImageTags.imageid "
                   " WHERE Images.status=1;", &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        tagID = (*it).toInt();
        ++it;

        QMap<int, int>::iterator it2 = tagsStatMap.find(tagID);

        if (it2 == tagsStatMap.end())
        {
            tagsStatMap.insert(tagID, 1);
        }
        else
        {
            it2.value()++;
        }
    }

    return tagsStatMap;
}

QMap<int, int> AlbumDB::getNumberOfImagesInTagProperties(const QString& property)
{
    QList<QVariant> values;
    QMap<int, int>  tagsStatMap;
    int             tagID, count;

    d->db->execSql("SELECT tagid, COUNT(*) FROM ImageTagProperties "
                   " LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                   " WHERE ImageTagProperties.property=? AND Images.status=1 "
                   " GROUP BY tagid;",
                   property, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        tagID = (*it).toInt();
        ++it;
        count = (*it).toInt();
        ++it;

        tagsStatMap[tagID] = count;
    }

    return tagsStatMap;
}

QMap<QString, int> AlbumDB::getFormatStatistics()
{
    return getFormatStatistics(DatabaseItem::UndefinedCategory);
}

QMap<QString, int> AlbumDB::getFormatStatistics(DatabaseItem::Category category)
{
    QMap<QString, int>  map;

    QString queryString = "SELECT COUNT(*), II.format "
                          "  FROM ImageInformation AS II "
                          "  INNER JOIN Images ON II.imageid=Images.id "
                          "  WHERE Images.status=1 ";

    if (category != DatabaseItem::UndefinedCategory)
    {
        queryString.append(QString("AND Images.category=%1").arg(category));
    }

    queryString.append(" GROUP BY II.format;");
    kDebug() << queryString;

    SqlQuery query = d->db->prepareQuery(queryString);

    if (d->db->exec(query))
    {
        while (query.next())
        {
            QString quantity = query.value(0).toString();
            QString format   = query.value(1).toString();

            if (format.isEmpty())
            {
                continue;
            }

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
    d->db->execSql(QString("SELECT id FROM Albums WHERE albumRoot=? AND relativePath=?;"),
                   albumRootId, folder, &values);

    int albumID = -1;

    if (values.isEmpty())
    {
        if (create)
        {
            albumID = addAlbum(albumRootId, folder, QString(), QDate::currentDate(), QString());
        }
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
    d->db->execSql(QString("SELECT id, relativePath FROM Albums WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?);"),
                   albumRootId, relativePath, (relativePath == "/" ? "/%" : QString(relativePath + QLatin1String("/%"))), &values);

    QList<int> albumIds;
    int id;
    QString albumRelativePath;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        id = (*it).toInt();
        ++it;
        QString albumRelativePath = (*it).toString();
        ++it;

        // bug #223050: The LIKE operator is case insensitive
        if (albumRelativePath.startsWith(relativePath))
        {
            albumIds << id;
        }
    }

    return albumIds;
}

QList<int> AlbumDB::getAlbumsOnAlbumRoot(int albumRootId)
{
    QList<QVariant> values;
    d->db->execSql(QString("SELECT id FROM Albums WHERE albumRoot=?;"),
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
                           qlonglong fileSize,
                           const QString& uniqueHash)
{
    QVariantList boundValues;
    boundValues << albumID << name << (int)status << (int)category
                << modificationDate.toString(Qt::ISODate) << fileSize << uniqueHash;

    QVariant id;
    d->db->execSql(QString("REPLACE INTO Images "
                           " ( album, name, status, category, modificationDate, fileSize, uniqueHash ) "
                           " VALUES (?,?,?,?,?,?,?);"),
                   boundValues,
                   0, &id);

    if (id.isNull())
    {
        return -1;
    }

    d->db->recordChangeset(ImageChangeset(id.toLongLong(), DatabaseFields::ImagesAll));
    d->db->recordChangeset(CollectionImageChangeset(id.toLongLong(), albumID, CollectionImageChangeset::Added));
    return id.toLongLong();
}

void AlbumDB::updateItem(qlonglong imageID, DatabaseItem::Category category,
                         const QDateTime& modificationDate,
                         qlonglong fileSize, const QString& uniqueHash)
{
    QVariantList boundValues;
    boundValues << category << modificationDate << fileSize << uniqueHash << imageID;
    d->db->execSql(QString("UPDATE Images SET category=?, modificationDate=?, fileSize=?, uniqueHash=? WHERE id=?;"),
                   boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Category
                                          | DatabaseFields::ModificationDate
                                          | DatabaseFields::FileSize
                                          | DatabaseFields::UniqueHash));
}

void AlbumDB::setItemStatus(qlonglong imageID, DatabaseItem::Status status)
{
    QVariantList boundValues;
    boundValues << (int)status << imageID;
    d->db->execSql(QString("UPDATE Images SET status=? WHERE id=?;"),
                   boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Status));
}

/*
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
*/

int AlbumDB::getItemAlbum(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT album FROM Images WHERE id=?;"),
                   imageID, &values);

    if (!values.isEmpty())
    {
        return values.first().toInt();
    }
    else
    {
        return 1;
    }
}

QString AlbumDB::getItemName(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT name FROM Images WHERE id=?;"),
                   imageID, &values);

    if (!values.isEmpty())
    {
        return values.first().toString();
    }
    else
    {
        return QString();
    }
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
    {
        return QStringList();
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(albumRootId);

    if (albumRootPath.isNull())
    {
        return QStringList();
    }

    QMap<QString, QVariant> bindingMap;
    bindingMap.insert(QString(":albumID"), albumID);

    switch (sortOrder)
    {
        case ByItemName:
            d->db->execDBAction(d->db->getDBAction(QString("getItemURLsInAlbumByItemName")), bindingMap, &values);
            break;

        case ByItemPath:
            // Don't collate on the path - this is to maintain the same behavior
            // that happens when sort order is "By Path"
            d->db->execDBAction(d->db->getDBAction(QString("getItemURLsInAlbumByItemPath")), bindingMap, &values);
            break;

        case ByItemDate:
            d->db->execDBAction(d->db->getDBAction(QString("getItemURLsInAlbumByItemDate")), bindingMap, &values);
            break;

        case ByItemRating:
            d->db->execDBAction(d->db->getDBAction(QString("getItemURLsInAlbumByItemRating")), bindingMap, &values);
            break;

        case NoItemSorting:
        default:
            d->db->execDBAction(d->db->getDBAction(QString("getItemURLsInAlbumNoItemSorting")), bindingMap, &values);
            break;
    }

    QStringList urls;
    QString     relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;

        if (relativePath == "/")
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + '/' + name;
        }
    }

    return urls;
}

QList<qlonglong> AlbumDB::getItemIDsInAlbum(int albumID)
{
    QList<qlonglong> itemIDs;
    QList<QVariant>  values;

    d->db->execSql(QString("SELECT id FROM Images WHERE album=?;"),
                   albumID, &values);

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
    {
        return QMap<qlonglong, QString>();
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(albumRootId);

    if (albumRootPath.isNull())
    {
        return QMap<qlonglong, QString>();
    }

    QMap<qlonglong, QString> itemsMap;
    QList<QVariant> values;

    d->db->execSql(QString("SELECT Images.id, Albums.relativePath, Images.name "
                           "FROM Images JOIN Albums ON Albums.id=Images.album "
                           "WHERE Albums.id=?;"),
                   albumID, &values);

    QString   path;
    qlonglong id;
    QString   relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        id = (*it).toLongLong();
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;

        if (relativePath == "/")
        {
            path = albumRootPath + relativePath + name;
        }
        else
        {
            path = albumRootPath + relativePath + '/' + name;
        }

        itemsMap.insert(id, path);
    };

    return itemsMap;
}

QList<ItemScanInfo> AlbumDB::getItemScanInfos(int albumID)
{
    QList<QVariant> values;

    d->db->execSql(QString("SELECT id, album, name, status, category, modificationDate, fileSize, uniqueHash "
                           "FROM Images WHERE album=?;"),
                   albumID,
                   &values);

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
                                 : QDateTime::fromString((*it).toString(), Qt::ISODate));
        ++it;
        info.fileSize         = (*it).toLongLong();
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

    d->db->execSql(QString("SELECT id, album, name, status, category, modificationDate, fileSize, uniqueHash "
                           "FROM Images WHERE id=?;"),
                   imageID,
                   &values);

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
                                 : QDateTime::fromString((*it).toString(), Qt::ISODate));
        ++it;
        info.fileSize         = (*it).toLongLong();
        ++it;
        info.uniqueHash       = (*it).toString();
        ++it;
    }

    return info;
}

QStringList AlbumDB::getItemURLsInTag(int tagID, bool recursive)
{
    QList<QVariant>         values;
    QString                 imagesIdClause;
    QMap<QString, QVariant> bindingMap;

    bindingMap.insert(QString(":tagID"), tagID);
    bindingMap.insert(QString(":tagID2"), tagID);

    if (recursive)
    {
        d->db->execDBAction(d->db->getDBAction(QString("GetItemURLsInTagRecursive")), bindingMap, &values);
    }
    else
    {
        d->db->execDBAction(d->db->getDBAction(QString("GetItemURLsInTag")), bindingMap, &values);
    }

    QStringList urls;
    QString     albumRootPath, relativePath, name;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        albumRootPath = CollectionManager::instance()->albumRootPath((*it).toInt());
        ++it;
        relativePath = (*it).toString();
        ++it;
        name = (*it).toString();
        ++it;

        if (relativePath == "/")
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + '/' + name;
        }
    }

    return urls;
}

QList<qlonglong> AlbumDB::getItemIDsInTag(int tagID, bool recursive)
{
    QList<qlonglong>        itemIDs;
    QList<QVariant>         values;
    QMap<QString, QVariant> parameters;

    parameters.insert(":tagPID", tagID);
    parameters.insert(":tagID",  tagID);

    if (recursive)
    {
        d->db->execDBAction(d->db->getDBAction(QString("getItemIDsInTagRecursive")), parameters, &values);
    }
    else
    {
        d->db->execDBAction(d->db->getDBAction(QString("getItemIDsInTag")), parameters, &values);
    }

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
    d->db->execSql(QString("SELECT relativePath from Albums WHERE id=?"),
                   albumID, &values);

    if (!values.isEmpty())
    {
        return values.first().toString();
    }
    else
    {
        return QString();
    }
}

int AlbumDB::getAlbumRootId(int albumID)
{
    QList<QVariant> values;
    d->db->execSql(QString("SELECT albumRoot FROM Albums WHERE id=?; "),
                   albumID, &values);

    if (!values.isEmpty())
    {
        return values.first().toInt();
    }
    else
    {
        return -1;
    }
}

QDate AlbumDB::getAlbumLowestDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql("SELECT MIN(creationDate) FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.album=? GROUP BY Images.album;",
                   albumID, &values);

    if (!values.isEmpty())
    {
        return QDate::fromString(values.first().toString(), Qt::ISODate);
    }
    else
    {
        return QDate();
    }
}

QDate AlbumDB::getAlbumHighestDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql("SELECT MAX(creationDate) FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.album=? GROUP BY Images.album;",
                   albumID , &values);

    if (!values.isEmpty())
    {
        return QDate::fromString(values.first().toString(), Qt::ISODate);
    }
    else
    {
        return QDate();
    }
}

QDate AlbumDB::getAlbumAverageDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql("SELECT creationDate FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.album=?;",
                   albumID , &values);

    QList<QDate> dates;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        QDateTime itemDateTime = (*it).isNull() ? QDateTime()
                                 : QDateTime::fromString((*it).toString(), Qt::ISODate);

        if (itemDateTime.isValid())
        {
            dates << itemDateTime.date();
        }
    }

    if (dates.isEmpty())
    {
        return QDate();
    }

    qint64 julianDays = 0;

    foreach(const QDate& date, dates)
    {
        julianDays += date.toJulianDay();
    }

    return QDate::fromJulianDay(julianDays / dates.size());
}

void AlbumDB::deleteItem(int albumID, const QString& file)
{
    qlonglong imageId = getImageId(albumID, file);

    d->db->execSql(QString("DELETE FROM Images WHERE id=?;"),
                   imageId);

    d->db->recordChangeset(CollectionImageChangeset(imageId, albumID, CollectionImageChangeset::Deleted));
}

void AlbumDB::removeItemsFromAlbum(int albumID, const QList<qlonglong>& ids_forInformation)
{
    d->db->execSql(QString("UPDATE Images SET status=?, album=NULL WHERE album=?;"),
                   (int)DatabaseItem::Removed, albumID);

    d->db->recordChangeset(CollectionImageChangeset(ids_forInformation, albumID, CollectionImageChangeset::RemovedAll));
}

void AlbumDB::removeItems(QList<qlonglong> itemIDs, const QList<int>& albumIDs)
{
    SqlQuery query = d->db->prepareQuery(QString("UPDATE Images SET status=?, album=NULL WHERE id=?;"));

    QVariantList imageIds;
    QVariantList status;

    foreach(const qlonglong& id, itemIDs)
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
    d->db->execSql(QString("DELETE FROM Images WHERE status=?;"),
                   (int)DatabaseItem::Removed);

    d->db->recordChangeset(CollectionImageChangeset(QList<qlonglong>(), QList<int>(), CollectionImageChangeset::RemovedDeleted));
}

/*
// This method is probably nonsense because a remove image no longer has an associated album
void AlbumDB::deleteRemovedItems(QList<int> albumIds)
{
    SqlQuery query = d->db->prepareQuery( QString("DELETE FROM Images WHERE status=? AND album=?;") );

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
*/

void AlbumDB::renameAlbum(int albumID, int newAlbumRoot, const QString& newRelativePath)
{
    int albumRoot        = getAlbumRootId(albumID);
    QString relativePath = getAlbumRelativePath(albumID);

    if (relativePath == newRelativePath && albumRoot == newAlbumRoot)
    {
        return;
    }

    // first delete any stale albums left behind at the destination of renaming
    QMap<QString, QVariant> parameters;
    parameters.insert(":albumRoot", newAlbumRoot);
    parameters.insert(":relativePath", newRelativePath);

    if (DatabaseCoreBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QString("deleteAlbumRootPath")), parameters))
    {
        return;
    }

    // now update the album
    d->db->execSql(QString("UPDATE Albums SET albumRoot=?, relativePath=? WHERE id=? AND albumRoot=?;"),
                   newAlbumRoot, newRelativePath, albumID, albumRoot);
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
    d->db->execSql(QString("UPDATE Tags SET name=? WHERE id=?;"),
                   name, tagID);
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Renamed));
}

void AlbumDB::moveItem(int srcAlbumID, const QString& srcName,
                       int dstAlbumID, const QString& dstName)
{
    // find id of src image
    qlonglong imageId = getImageId(srcAlbumID, srcName);

    if (imageId == -1)
    {
        return;
    }

    // first delete any stale database entries (for destination) if any
    deleteItem(dstAlbumID, dstName);

    d->db->execSql(QString("UPDATE Images SET album=?, name=? "
                           "WHERE id=?;"),
                   dstAlbumID, dstName, imageId);
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
    {
        return -1;
    }

    // check for src == dest
    if (srcAlbumID == dstAlbumID && srcName == dstName)
    {
        return srcId;
    }

    // first delete any stale database entries if any
    deleteItem(dstAlbumID, dstName);

    // copy entry in Images table
    QVariant id;
    d->db->execSql(QString("INSERT INTO Images "
                           " ( album, name, status, category, modificationDate, fileSize, uniqueHash ) "
                           " SELECT ?, ?, status, category, modificationDate, fileSize, uniqueHash "
                           "  FROM Images WHERE id=?;"),
                   dstAlbumID, dstName, srcId,
                   0, &id);

    if (id.isNull())
    {
        return -1;
    }

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

    d->db->execSql(QString("INSERT INTO ImageHaarMatrix "
                           " (imageid, modificationDate, uniqueHash, matrix) "
                           "SELECT ?, modificationDate, uniqueHash, matrix "
                           "FROM ImageHaarMatrix WHERE imageid=?;"),
                   dstId, srcId);

    d->db->execSql(QString("INSERT INTO ImageInformation "
                           " (imageid, rating, creationDate, digitizationDate, orientation, "
                           "  width, height, format, colorDepth, colorModel) "
                           "SELECT ?, rating, creationDate, digitizationDate, orientation, "
                           "  width, height, format, colorDepth, colorModel "
                           "FROM ImageInformation WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageInformationAll;

    d->db->execSql(QString("INSERT INTO ImageMetadata "
                           " (imageid, make, model, lens, aperture, focalLength, focalLength35, "
                           "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                           "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) "
                           "SELECT ?, make, model, lens, aperture, focalLength, focalLength35, "
                           "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                           "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory "
                           "FROM ImageMetadata WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageMetadataAll;

    d->db->execSql(QString("INSERT INTO VideoMetadata "
                           " (imageid, aspectRatio, audioBitRate, audioChannelType, audioCompressor, duration, frameRate, "
                           "  videoCodec) "
                           "SELECT ?, aspectRatio, audioBitRate, audioChannelType, audioCompressor, duration, frameRate, "
                           "  videoCodec "
                           "FROM VideoMetadata WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::VideoMetadataAll;

    d->db->execSql(QString("INSERT INTO ImagePositions "
                           " (imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                           "  altitude, orientation, tilt, roll, accuracy, description) "
                           "SELECT ?, latitude, latitudeNumber, longitude, longitudeNumber, "
                           "  altitude, orientation, tilt, roll, accuracy, description "
                           "FROM ImagePositions WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImagePositionsAll;

    d->db->execSql(QString("INSERT INTO ImageComments "
                           " (imageid, type, language, author, date, comment) "
                           "SELECT ?, type, language, author, date, comment "
                           "FROM ImageComments WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageCommentsAll;

    d->db->execSql(QString("INSERT INTO ImageCopyright "
                           " (imageid, property, value, extraValue) "
                           "SELECT ?, property, value, extraValue "
                           "FROM ImageCopyright WHERE imageid=?;"),
                   dstId, srcId);

    d->db->execSql(QString("INSERT INTO ImageHistory "
                           " (imageid, uuid, history) "
                           "SELECT ?, uuid, history "
                           "FROM ImageHistory WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageHistoryInfoAll;

    d->db->execSql(QString("INSERT INTO ImageRelations "
                           " (subject, object, type) "
                           "SELECT ?, object, type "
                           "FROM ImageRelations WHERE subject=?;"),
                   dstId, srcId);
    d->db->execSql(QString("INSERT INTO ImageRelations "
                           " (subject, object, type) "
                           "SELECT subject, ?, type "
                           "FROM ImageRelations WHERE object=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageRelations;

    d->db->recordChangeset(ImageChangeset(dstId, fields));

    copyImageTags(srcId, dstId);
    copyImageProperties(srcId, dstId);
}

void AlbumDB::copyImageProperties(qlonglong srcId, qlonglong dstId)
{
    d->db->execSql(QString("INSERT INTO ImageProperties "
                           " (imageid, property, value) "
                           "SELECT ?, property, value "
                           "FROM ImageProperties WHERE imageid=?;"),
                   dstId, srcId);
}

void AlbumDB::copyImageTags(qlonglong srcId, qlonglong dstId)
{
    d->db->execSql(QString("INSERT INTO ImageTags "
                           " (imageid, tagid) "
                           "SELECT ?, tagid "
                           "FROM ImageTags WHERE imageid=?;"),
                   dstId, srcId);

    d->db->execSql(QString("INSERT INTO ImageTagProperties "
                           " (imageid, tagid, property, value) "
                           "SELECT ?, tagid, property, value "
                           "FROM ImageTagProperties WHERE imageid=?;"),
                   dstId, srcId);

    // leave empty tag list for now
    d->db->recordChangeset(ImageTagChangeset(dstId, QList<int>(),
                                             ImageTagChangeset::Added));

    d->db->recordChangeset(ImageTagChangeset(dstId, QList<int>(),
                                             ImageTagChangeset::PropertiesChanged));
}

bool AlbumDB::copyAlbumProperties(int srcAlbumID, int dstAlbumID)
{
    if (srcAlbumID == dstAlbumID)
    {
        return true;
    }

    QList<QVariant> values;
    d->db->execSql(QString("SELECT date, caption, collection, icon "
                           "FROM Albums WHERE id=?;"),
                   srcAlbumID,
                   &values);

    if (values.isEmpty())
    {
        kWarning() << " src album ID " << srcAlbumID << " does not exist";
        return false;
    }

    QList<QVariant> boundValues;
    boundValues << values.at(0) << values.at(1) << values.at(2) << values.at(3);
    boundValues << dstAlbumID;

    //shouldn't we have a ; at the end of the query?
    d->db->execSql(QString("UPDATE Albums SET date=?, caption=?, "
                           "collection=?, icon=? WHERE id=?"),
                   boundValues);
    return true;
}

QList<QVariant> AlbumDB::getImageIdsFromArea(qreal lat1, qreal lat2, qreal lng1, qreal lng2, int /*sortMode*/,
                                             const QString& /*sortBy*/)
{
    QList<QVariant> values;
    QList<QVariant> boundValues;
    boundValues << lat1 << lat2 << lng1 << lng2;

    //DatabaseAccess access;

    d->db->execSql(QString("Select ImageInformation.imageid, ImageInformation.rating, ImagePositions.latitudeNumber, ImagePositions.longitudeNumber"
                           " FROM ImageInformation INNER JOIN ImagePositions"
                           " ON ImageInformation.imageid = ImagePositions.imageid"
                           " WHERE (ImagePositions.latitudeNumber>? AND ImagePositions.latitudeNumber<?)"
                           " AND (ImagePositions.longitudeNumber>? AND ImagePositions.longitudeNumber<?);"),
                   boundValues, &values);

    return values;
}

void AlbumDB::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->recentlyAssignedTags = group.readEntry(d->configRecentlyUsedTags, QList<int>());
}

void AlbumDB::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configRecentlyUsedTags, d->recentlyAssignedTags);
}

}  // namespace Digikam
