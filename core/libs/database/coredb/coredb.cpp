/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description : Core database interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "coredb.h"

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

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "coredbbackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "dbengineactiontype.h"
#include "tagscache.h"
#include "album.h"

namespace Digikam
{

class CoreDB::Private
{

public:

    Private() :
        db(0),
        uniqueHashVersion(-1)
    {
    }

    static const QString configGroupName;
    static const QString configRecentlyUsedTags;

    CoreDbBackend*       db;
    QList<int>           recentlyAssignedTags;

    int                  uniqueHashVersion;

public:

    QString constructRelatedImagesSQL(bool fromOrTo, DatabaseRelation::Type type, bool boolean);
    QList<qlonglong> execRelatedImagesQuery(DbEngineSqlQuery& query, qlonglong id, DatabaseRelation::Type type);
};

const QString CoreDB::Private::configGroupName(QLatin1String("CoreDB Settings"));
const QString CoreDB::Private::configRecentlyUsedTags(QLatin1String("Recently Used Tags"));

QString CoreDB::Private::constructRelatedImagesSQL(bool fromOrTo, DatabaseRelation::Type type, bool boolean)
{
    QString sql;

    if (fromOrTo)
    {
        sql = QString::fromUtf8("SELECT object FROM ImageRelations "
              "INNER JOIN Images ON ImageRelations.object=Images.id "
              "WHERE subject=? %1 AND status!=3 %2;");
    }
    else
    {
        sql = QString::fromUtf8("SELECT subject FROM ImageRelations "
              "INNER JOIN Images ON ImageRelations.subject=Images.id "
              "WHERE object=? %1 AND status!=3 %2;");
    }

    if (type != DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString::fromUtf8("AND type=?"));
    }
    else
    {
        sql = sql.arg(QString());
    }

    if (boolean)
    {
        sql = sql.arg(QString::fromUtf8("LIMIT 1"));
    }
    else
    {
        sql = sql.arg(QString());
    }

    return sql;
}

QList<qlonglong> CoreDB::Private::execRelatedImagesQuery(DbEngineSqlQuery& query, qlonglong id, DatabaseRelation::Type type)
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

CoreDB::CoreDB(CoreDbBackend* const backend)
    : d(new Private)
{
    d->db = backend;
    readSettings();
}

CoreDB::~CoreDB()
{
    writeSettings();
    delete d;
}

QList<AlbumRootInfo> CoreDB::getAlbumRoots()
{
    QList<AlbumRootInfo> list;
    QList<QVariant>      values;

    d->db->execSql(QString::fromUtf8("SELECT id, label, status, type, identifier, specificPath FROM AlbumRoots;"), &values);

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

int CoreDB::addAlbumRoot(AlbumRoot::Type type, const QString& identifier, const QString& specificPath, const QString& label)
{
    QVariant id;
    d->db->execSql(QString::fromUtf8("REPLACE INTO AlbumRoots (type, label, status, identifier, specificPath) "
                           "VALUES(?, ?, 0, ?, ?);"),
                   (int)type, label, identifier, specificPath, 0, &id);

    d->db->recordChangeset(AlbumRootChangeset(id.toInt(), AlbumRootChangeset::Added));
    return id.toInt();
}

void CoreDB::deleteAlbumRoot(int rootId)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM AlbumRoots WHERE id=?;"),
                   rootId);
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":albumRoot"), rootId);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumRoot")), parameters))
    {
        return;
    }

    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::Deleted));
}

void CoreDB::migrateAlbumRoot(int rootId, const QString& identifier)
{
    d->db->execSql(QString::fromUtf8("UPDATE AlbumRoots SET identifier=? WHERE id=?;"),
                   identifier, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

void CoreDB::setAlbumRootLabel(int rootId, const QString& newLabel)
{
    d->db->execSql(QString::fromUtf8("UPDATE AlbumRoots SET label=? WHERE id=?;"),
                   newLabel, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

void CoreDB::changeAlbumRootType(int rootId, AlbumRoot::Type newType)
{
    d->db->execSql(QString::fromUtf8("UPDATE AlbumRoots SET type=? WHERE id=?;"),
                   (int)newType, rootId);
    d->db->recordChangeset(AlbumRootChangeset(rootId, AlbumRootChangeset::PropertiesChanged));
}

AlbumInfo::List CoreDB::scanAlbums()
{
    AlbumInfo::List aList;

    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT albumRoot, id, relativePath, date, caption, collection, icon FROM Albums "
                   " WHERE albumRoot != 0;"), // exclude stale albums
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

TagInfo::List CoreDB::scanTags()
{
    TagInfo::List tList;

    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id, pid, name, icon, iconkde FROM Tags;"), &values);

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

TagInfo CoreDB::getTagInfo(int tagId)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id, pid, name, icon, iconkde WHERE id=? FROM Tags;"), tagId, &values);

    TagInfo info;

    if (!values.isEmpty() && values.size() == 5)
    {
        QList<QVariant>::const_iterator it = values.constBegin();

        info.id = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = (*it).toString();
        ++it;
        info.iconId = (*it).toLongLong();
        ++it;
        info.icon   = (*it).toString();
        ++it;
    }

    return info;
}

SearchInfo::List CoreDB::scanSearches()
{
    SearchInfo::List searchList;
    QList<QVariant>  values;

    d->db->execSql(QString::fromUtf8("SELECT id, type, name, query FROM Searches;"), &values);

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

QList<AlbumShortInfo> CoreDB::getAlbumShortInfos()
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT Albums.id, Albums.relativePath, Albums.albumRoot from Albums ORDER BY Albums.id; "),
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

QList<TagShortInfo> CoreDB::getTagShortInfos()
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT id, pid, name FROM Tags ORDER BY id;"), &values);

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
QStringList CoreDB::getSubalbumsForPath(const QString& albumRoot,
                                         const QString& path,
                                         bool onlyDirectSubalbums)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
        return QStringList();

    QString subURL = path;

    if (!path.endsWith(QLatin1String("/")))
        subURL += QLatin1Char('/');

    subURL = (subURL);

    QList<QVariant> values;

    if (onlyDirectSubalbums)
    {
        d->db->execSql( QString::fromUtf8("SELECT relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE '") +
                        subURL + QString::fromUtf8("%' ") + QString::fromUtf8("AND relativePath NOT LIKE '") +
                        subURL + QString::fromUtf8("%/%'; "),
                        location.id(),
                        &values );
    }
    else
    {
        d->db->execSql( QString::fromUtf8("SELECT relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE '") +
                        subURL + QString::fromUtf8("%'; "),
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
int CoreDB::addAlbum(const QString& albumRoot, const QString& relativePath,
                      const QString& caption,
                      const QDate& date, const QString& collection)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
        return -1;

    return addAlbum(location.id(), relativePath, caption, date, collection);
}
*/

int CoreDB::addAlbum(int albumRootId, const QString& relativePath,
                      const QString& caption,
                      const QDate& date, const QString& collection)
{
    QVariant        id;
    QList<QVariant> boundValues;

    boundValues << albumRootId << relativePath << date.toString(Qt::ISODate) << caption << collection;

    d->db->execSql(QString::fromUtf8("REPLACE INTO Albums (albumRoot, relativePath, date, caption, collection) "
                                     "VALUES(?, ?, ?, ?, ?);"),
                   boundValues, 0, &id);

    d->db->recordChangeset(AlbumChangeset(id.toInt(), AlbumChangeset::Added));
    return id.toInt();
}

void CoreDB::setAlbumCaption(int albumID, const QString& caption)
{
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET caption=? WHERE id=?;"),
                   caption, albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void CoreDB::setAlbumCategory(int albumID, const QString& category)
{
    // TODO : change "collection" propertie in DB ALbum table to "category"
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET collection=? WHERE id=?;"),
                   category, albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void CoreDB::setAlbumDate(int albumID, const QDate& date)
{
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET date=? WHERE id=?;"),
                   date.toString(Qt::ISODate),
                   albumID);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void CoreDB::setAlbumIcon(int albumID, qlonglong iconID)
{
    if (iconID == 0)
    {
        d->db->execSql(QString::fromUtf8("UPDATE Albums SET icon=NULL WHERE id=?;"),
                       albumID);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("UPDATE Albums SET icon=? WHERE id=?;"),
                       iconID,
                       albumID);
    }
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::PropertiesChanged));
}

void CoreDB::deleteAlbum(int albumID)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":albumId"), albumID);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumID")), parameters))
    {
        return;
    }

    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Deleted));
}

void CoreDB::makeStaleAlbum(int albumID)
{
    // We need to work around the table constraint, no we want to delete older stale albums with
    // the same relativePath, and adjust relativePaths depending on albumRoot.
    QList<QVariant> values;

    // retrieve information
    d->db->execSql(QString::fromUtf8("SELECT Albums.albumRoot, Albums.relativePath from Albums WHERE id=?;"),
                   albumID, &values);

    if (values.isEmpty())
    {
        return;
    }

    // prepend albumRootId to relativePath. relativePath is unused and officially undefined after this call.
    QString newRelativePath = values.at(0).toString() + QLatin1Char('-') + values.at(1).toString();

    // delete older stale albums
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":albumRoot"), 0);
    parameters.insert(QLatin1String(":relativePath"), newRelativePath);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumRootPath")), parameters))
    {
        return;
    }

    // now do our update
    d->db->setForeignKeyChecks(false);
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET albumRoot=0, relativePath=? WHERE id=?;"),
                   newRelativePath, albumID);

    // for now, we make no distinction to deleteAlbums wrt to changeset
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Deleted));
    d->db->setForeignKeyChecks(true);
}

void CoreDB::deleteStaleAlbums()
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":albumRoot"), 0);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("deleteAlbumRoot")), parameters))
    {
        return;
    }

    // deliberately no changeset here, is done above
}

int CoreDB::addTag(int parentTagID, const QString& name, const QString& iconKDE,
                    qlonglong iconID)
{
    QVariant                id;
    QMap<QString, QVariant> parameters;

    parameters.insert(QLatin1String(":tagPID"), parentTagID);
    parameters.insert(QLatin1String(":tagname"), name);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QLatin1String("InsertTag")), parameters, 0 , &id))
    {
        return -1;
    }

    if (!iconKDE.isEmpty())
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET iconkde=? WHERE id=?;"),
                       iconKDE,
                       id.toInt());
    }
    else if (iconID == 0)
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET icon=NULL WHERE id=?;"),
                       id.toInt());
    }
    else
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET icon=? WHERE id=?;"),
                       iconID,
                       id.toInt());
    }

    d->db->recordChangeset(TagChangeset(id.toInt(), TagChangeset::Added));
    return id.toInt();
}

void CoreDB::deleteTag(int tagID)
{
    /*
    QString("DELETE FROM Tags WHERE id=?;"), tagID
    */

    QMap<QString, QVariant> bindingMap;
    bindingMap.insert(QLatin1String(":tagID"), tagID);

    d->db->execDBAction(d->db->getDBAction(QLatin1String("DeleteTag")), bindingMap);
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Deleted));
}

void CoreDB::setTagIcon(int tagID, const QString& iconKDE, qlonglong iconID)
{
    int     _iconID  = iconKDE.isEmpty() ? iconID : 0;
    QString _iconKDE = iconKDE;

    if (iconKDE.isEmpty() || iconKDE.toLower() == QLatin1String("tag"))
    {
        _iconKDE.clear();
    }

    if (_iconID == 0)
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET iconkde=?, icon=NULL WHERE id=?;"),
                       _iconKDE, tagID);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET iconkde=?, icon=? WHERE id=?;"),
                       _iconKDE, _iconID, tagID);
    }

    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::IconChanged));
}

void CoreDB::setTagParentID(int tagID, int newParentTagID)
{
    if (d->db->databaseType() == BdEngineBackend::DbType::SQLite)
    {
        d->db->execSql(QString::fromUtf8("UPDATE OR REPLACE Tags SET pid=? WHERE id=?;"),
                       newParentTagID, tagID);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("UPDATE Tags SET pid=? WHERE id=?;"),
                       newParentTagID, tagID);

        // NOTE: Update the Mysql TagsTree table which is used only in some search SQL queries (See lft/rgt tag ID properties).
        // In SQlite, it is nicely maintained by Triggers.
        // With MySQL, this did not work for some reason, and we patch a tree structure mimics in a different way.
        QMap<QString, QVariant> bindingMap;
        bindingMap.insert(QLatin1String(":tagID"),     tagID);
        bindingMap.insert(QLatin1String(":newTagPID"), newParentTagID);

        d->db->execDBAction(d->db->getDBAction(QLatin1String("MoveTagTree")), bindingMap);
   }

    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Reparented));
}

QList<TagProperty> CoreDB::getTagProperties(int tagId)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT property, value FROM TagProperties WHERE tagid=?;"),
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

QList<TagProperty> CoreDB::getTagProperties(const QString& property)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT tagid, property, value FROM TagProperties WHERE property=?;"),
                   property, &values);

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

QList<TagProperty> CoreDB::getTagProperties()
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT tagid, property, value FROM TagProperties ORDER BY tagid, property;"), &values);

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

QList< int > CoreDB::getTagsWithProperty(const QString& property)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT DISTINCT tagid FROM TagProperties WHERE property=?;"),
                   property, &values);

    QList<int> tagIds;

    foreach(const QVariant& var, values)
    {
        tagIds << var.toInt();
    }

    return tagIds;
}

void CoreDB::addTagProperty(int tagId, const QString& property, const QString& value)
{
    d->db->execSql(QString::fromUtf8("INSERT INTO TagProperties (tagid, property, value) VALUES(?, ?, ?);"),
                   tagId, property, value);

    d->db->recordChangeset(TagChangeset(tagId, TagChangeset::PropertiesChanged));
}

void CoreDB::addTagProperty(const TagProperty& property)
{
    addTagProperty(property.tagId, property.property, property.value);
}

void CoreDB::removeTagProperties(int tagId, const QString& property, const QString& value)
{
    if (property.isNull())
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM TagProperties WHERE tagid=?;"),
                       tagId);
    }
    else if (value.isNull())
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM TagProperties WHERE tagid=? AND property=?;"),
                       tagId, property);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM TagProperties WHERE tagid=? AND property=? AND value=?;"),
                       tagId, property, value);
    }

    d->db->recordChangeset(TagChangeset(tagId, TagChangeset::PropertiesChanged));
}

int CoreDB::addSearch(DatabaseSearch::Type type, const QString& name, const QString& query)
{
    QVariant id;

    if (!d->db->execSql(QString::fromUtf8("INSERT INTO Searches (type, name, query) VALUES(?, ?, ?);"),
                        type, name, query, 0, &id))
    {
        return -1;
    }

    d->db->recordChangeset(SearchChangeset(id.toInt(), SearchChangeset::Added));
    return id.toInt();
}

void CoreDB::updateSearch(int searchID, DatabaseSearch::Type type,
                           const QString& name, const QString& query)
{
    d->db->execSql(QString::fromUtf8("UPDATE Searches SET type=?, name=?, query=? WHERE id=?"),
                   type, name, query, searchID);
    d->db->recordChangeset(SearchChangeset(searchID, SearchChangeset::Changed));
}

void CoreDB::deleteSearch(int searchID)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Searches WHERE id=?"),
                   searchID);
    d->db->recordChangeset(SearchChangeset(searchID, SearchChangeset::Deleted));
}

void CoreDB::deleteSearches(DatabaseSearch::Type type)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Searches WHERE type=?"),
                   type);
    d->db->recordChangeset(SearchChangeset(0, SearchChangeset::Deleted));
}

QString CoreDB::getSearchQuery(int searchId)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT query FROM Searches WHERE id=?;"),
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

SearchInfo CoreDB::getSearchInfo(int searchId)
{
    SearchInfo info;

    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id, type, name, query FROM Searches WHERE id=?;"),
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

void CoreDB::setSetting(const QString& keyword,
                         const QString& value)
{
    d->db->execSql(QString::fromUtf8("REPLACE into Settings VALUES (?,?);"),
                   keyword, value);
}

QString CoreDB::getSetting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT value FROM Settings "
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
static QStringList joinMainAndUserFilterString(const QChar& sep, const QString& filter,
                                               const QString& userFilter)
{
    QSet<QString> filterSet;
    QStringList   userFilterList;
    QStringList   sortedList;

    filterSet      = filter.split(sep, QString::SkipEmptyParts).toSet();
    userFilterList = userFilter.split(sep, QString::SkipEmptyParts);

    foreach(const QString& userFormat, userFilterList)
    {
        if (userFormat.startsWith(QLatin1Char('-')))
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

void CoreDB::getFilterSettings(QStringList* imageFilter, QStringList* videoFilter, QStringList* audioFilter)
{
    QString imageFormats, videoFormats, audioFormats, userImageFormats, userVideoFormats, userAudioFormats;

    if (imageFilter)
    {
        imageFormats     = getSetting(QLatin1String("databaseImageFormats"));
        userImageFormats = getSetting(QLatin1String("databaseUserImageFormats"));
        *imageFilter     = joinMainAndUserFilterString(QLatin1Char(';'), imageFormats, userImageFormats);
    }

    if (videoFilter)
    {
        videoFormats     = getSetting(QLatin1String("databaseVideoFormats"));
        userVideoFormats = getSetting(QLatin1String("databaseUserVideoFormats"));
        *videoFilter     = joinMainAndUserFilterString(QLatin1Char(';'), videoFormats, userVideoFormats);
    }

    if (audioFilter)
    {
        audioFormats     = getSetting(QLatin1String("databaseAudioFormats"));
        userAudioFormats = getSetting(QLatin1String("databaseUserAudioFormats"));
        *audioFilter     = joinMainAndUserFilterString(QLatin1Char(';'), audioFormats, userAudioFormats);
    }
}

void CoreDB::getUserFilterSettings(QString* imageFilterString, QString* videoFilterString, QString* audioFilterString)
{
    if (imageFilterString)
    {
        *imageFilterString = getSetting(QLatin1String("databaseUserImageFormats"));
    }

    if (videoFilterString)
    {
        *videoFilterString = getSetting(QLatin1String("databaseUserVideoFormats"));
    }

    if (audioFilterString)
    {
        *audioFilterString = getSetting(QLatin1String("databaseUserAudioFormats"));
    }
}

void CoreDB::getUserIgnoreDirectoryFilterSettings(QString* ignoreDirectoryFilterString)
{
    *ignoreDirectoryFilterString = getSetting(QLatin1String("databaseUserIgnoreDirectoryFormats"));
}

void CoreDB::getIgnoreDirectoryFilterSettings(QStringList* ignoreDirectoryFilter)
{
    QString ignoreDirectoryFormats, userIgnoreDirectoryFormats;

    ignoreDirectoryFormats     = getSetting(QLatin1String("databaseIgnoreDirectoryFormats"));
    userIgnoreDirectoryFormats = getSetting(QLatin1String("databaseUserIgnoreDirectoryFormats"));
    *ignoreDirectoryFilter     = joinMainAndUserFilterString(QLatin1Char(';'), ignoreDirectoryFormats, userIgnoreDirectoryFormats);
}

void CoreDB::setFilterSettings(const QStringList& imageFilter, const QStringList& videoFilter, const QStringList& audioFilter)
{
    setSetting(QLatin1String("databaseImageFormats"), imageFilter.join(QLatin1Char(';')));
    setSetting(QLatin1String("databaseVideoFormats"), videoFilter.join(QLatin1Char(';')));
    setSetting(QLatin1String("databaseAudioFormats"), audioFilter.join(QLatin1Char(';')));
}

void CoreDB::setIgnoreDirectoryFilterSettings(const QStringList& ignoreDirectoryFilter)
{
    setSetting(QLatin1String("databaseIgnoreDirectoryFormats"), ignoreDirectoryFilter.join(QLatin1Char(';')));
}

void CoreDB::setUserFilterSettings(const QStringList& imageFilter,
                                   const QStringList& videoFilter,
                                   const QStringList& audioFilter)
{
    setSetting(QLatin1String("databaseUserImageFormats"), imageFilter.join(QLatin1Char(';')));
    setSetting(QLatin1String("databaseUserVideoFormats"), videoFilter.join(QLatin1Char(';')));
    setSetting(QLatin1String("databaseUserAudioFormats"), audioFilter.join(QLatin1Char(';')));
}

void CoreDB::setUserIgnoreDirectoryFilterSettings(const QStringList& ignoreDirectoryFilters)
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "CoreDB::setUserIgnoreDirectoryFilterSettings. "
                                     "ignoreDirectoryFilterString: "
                                  << ignoreDirectoryFilters.join(QLatin1Char(';'));

    setSetting(QLatin1String("databaseUserIgnoreDirectoryFormats"), ignoreDirectoryFilters.join(QLatin1Char(';')));
}

QUuid CoreDB::databaseUuid()
{
    QString uuidString = getSetting(QLatin1String("databaseUUID"));
    QUuid uuid         = QUuid(uuidString);

    if (uuidString.isNull() || uuid.isNull())
    {
        uuid = QUuid::createUuid();
        setSetting(QLatin1String("databaseUUID"), uuid.toString());
    }

    return uuid;
}

int CoreDB::getUniqueHashVersion()
{
    if (d->uniqueHashVersion == -1)
    {
        QString v = getSetting(QLatin1String("uniqueHashVersion"));

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

bool CoreDB::isUniqueHashV2()
{
    return (getUniqueHashVersion() == 2);
}

void CoreDB::setUniqueHashVersion(int version)
{
    d->uniqueHashVersion = version;
    setSetting(QLatin1String("uniqueHashVersion"), QString::number(d->uniqueHashVersion));
}

/*
QString CoreDB::getItemCaption(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString::fromUtf8("SELECT caption FROM Images "
                            "WHERE id=?;"),
                    imageID, &values );

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

QString CoreDB::getItemCaption(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql( QString::fromUtf8("SELECT caption FROM Images "
                            "WHERE dirid=? AND name=?;"),
                    albumID,
                    name,
                    &values );

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

QDateTime CoreDB::getItemDate(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString::fromUtf8("SELECT datetime FROM Images "
                            "WHERE id=?;"),
                    imageID,
                    &values );

    if (!values.isEmpty())
        return QDateTime::fromString(values.first().toString(), Qt::ISODate);
    else
        return QDateTime();
}

QDateTime CoreDB::getItemDate(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql( QString::fromUtf8("SELECT datetime FROM Images "
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

qlonglong CoreDB::getImageId(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
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

QList<qlonglong> CoreDB::getImageIds(int albumID, const QString& name, DatabaseItem::Status status)
{
    QList<QVariant> values;

    if (albumID == -1)
    {
        d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                         "WHERE album IS NULL AND name=? AND status=?;"),
                                         name,
                                         status,
                                         &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                         "WHERE album=? AND name=? AND status=?;"),
                                         albumID,
                                         name,
                                         status,
                                         &values);
    }

    QList<qlonglong> items;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        items << it->toLongLong();
    }

    return items;
}

QList<qlonglong> CoreDB::getImageIds(DatabaseItem::Status status)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                     "WHERE status=?;"),
                                     status,
                                     &values);

    QList<qlonglong> imageIds;

    foreach(QVariant object, values)
    {
        imageIds << object.toLongLong();
    }

    return imageIds;
}

QList<qlonglong> CoreDB::getImageIds(DatabaseItem::Status status, DatabaseItem::Category category)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                             "WHERE status=? AND category=?;"),
                   status, category,
                   &values);

    QList<qlonglong> imageIds;

    foreach(QVariant object, values)
    {
        imageIds << object.toLongLong();
    }

    return imageIds;
}

qlonglong CoreDB::getImageId(int albumID, const QString& name,
                             DatabaseItem::Status status,
                             DatabaseItem::Category category,
                             const QDateTime& modificationDate,
                             qlonglong fileSize,
                             const QString& uniqueHash)
{
    QList<QVariant> values;
    QVariantList boundValues;

    // Add the standard bindings
    boundValues << name << (int)status << (int)category
                << modificationDate.toString(Qt::ISODate) << fileSize << uniqueHash;

    // If the album id is -1, no album is assigned. Get all images with NULL album
    if (albumID == -1)
    {
        d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                         "WHERE name=? AND status=? "
                                         "AND category=? AND modificationDate=? "
                                         "AND fileSize=? AND uniqueHash=? "
                                         "AND album IS NULL;"),
                                         boundValues,
                                         &values);
    }
    else
    {
        boundValues << albumID;

        d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                                         "WHERE name=? AND status=? "
                                         "AND category=? AND modificationDate=? "
                                         "AND fileSize=? AND uniqueHash=?; "
                                         "AND album=?;"),
                                         boundValues,
                                         &values);
    }

    if ( values.isEmpty() || ( values.size() > 1 ) )
    {
        return -1;
    }
    else
    {
        return values.first().toLongLong();
    }
}

QStringList CoreDB::getItemTagNames(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT name FROM Tags \n "
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

QList<int> CoreDB::getItemTagIDs(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT tagid FROM ImageTags WHERE imageID=?;"),
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

QVector<QList<int> > CoreDB::getItemsTagIDs(const QList<qlonglong> imageIds)
{
    if (imageIds.isEmpty())
    {
        return QVector<QList<int> >();
    }

    QVector<QList<int> > results(imageIds.size());
    DbEngineSqlQuery query = d->db->prepareQuery(QString::fromUtf8("SELECT tagid FROM ImageTags WHERE imageID=?;"));
    QVariantList values;

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

QList<ImageTagProperty> CoreDB::getImageTagProperties(qlonglong imageId, int tagId)
{
    QList<QVariant> values;

    if (tagId == -1)
    {
        d->db->execSql(QString::fromUtf8("SELECT tagid, property, value FROM ImageTagProperties "
                               "WHERE imageid=?;"),
                       imageId,
                       &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT tagid, property, value FROM ImageTagProperties "
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

QList<int> CoreDB::getTagIdsWithProperties(qlonglong imageId)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT DISTINCT tagid FROM ImageTagProperties WHERE imageid=?;"),
                   imageId,
                   &values);

    QList<int> tagIds;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        tagIds << (*it).toInt();
    }

    return tagIds;
}

void CoreDB::addImageTagProperty(qlonglong imageId, int tagId, const QString& property, const QString& value)
{
    d->db->execSql(QString::fromUtf8("INSERT INTO ImageTagProperties (imageid, tagid, property, value) VALUES(?, ?, ?, ?);"),
                   imageId, tagId, property, value);

    d->db->recordChangeset(ImageTagChangeset(imageId, tagId, ImageTagChangeset::PropertiesChanged));
}

void CoreDB::addImageTagProperty(const ImageTagProperty& property)
{
    addImageTagProperty(property.imageId, property.tagId, property.property, property.value);
}

void CoreDB::removeImageTagProperties(qlonglong imageId, int tagId, const QString& property,
                                       const QString& value)
{
    if (tagId == -1)
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties WHERE imageid=?;"),
                       imageId);
    }
    else if (property.isNull())
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties WHERE imageid=? AND tagid=?;"),
                       imageId, tagId);
    }
    else if (value.isNull())
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties WHERE imageid=? AND tagid=? AND property=?;"),
                       imageId, tagId, property);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageTagProperties WHERE imageid=? AND tagid=? AND property=? AND value=?;"),
                       imageId, tagId, property, value);
    }

    d->db->recordChangeset(ImageTagChangeset(imageId, tagId, ImageTagChangeset::PropertiesChanged));
}

ItemShortInfo CoreDB::getItemShortInfo(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT Images.name, Albums.albumRoot, Albums.relativePath, Albums.id "
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

ItemShortInfo CoreDB::getItemShortInfo(int albumRootId, const QString& relativePath, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT Images.id, Albums.id "
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

bool CoreDB::hasTags(const QList<qlonglong>& imageIDList)
{
    QList<int> ids;

    if (imageIDList.isEmpty())
    {
        return false;
    }

    QList<QVariant> values;
    QList<QVariant> boundValues;

    QString sql = QString::fromUtf8("SELECT count(tagid) FROM ImageTags "
                                    "WHERE imageid=? ");
    boundValues << imageIDList.first();

    QList<qlonglong>::const_iterator it = imageIDList.constBegin();
    ++it;

    for (; it != imageIDList.constEnd(); ++it)
    {
        sql += QString::fromUtf8(" OR imageid=? ");
        boundValues << (*it);
    }

    sql += QString::fromUtf8(";");
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

QList<int> CoreDB::getItemCommonTagIDs(const QList<qlonglong>& imageIDList)
{
    QList<int> ids;

    if (imageIDList.isEmpty())
    {
        return ids;
    }

    QList<QVariant> values;
    QList<QVariant> boundValues;

    QString sql = QString::fromUtf8("SELECT DISTINCT tagid FROM ImageTags "
                                    "WHERE imageid=? ");
    boundValues << imageIDList.first();

    QList<qlonglong>::const_iterator it = imageIDList.constBegin();
    ++it;

    for (; it != imageIDList.constEnd(); ++it)
    {
        sql += QString::fromUtf8(" OR imageid=? ");
        boundValues << (*it);
    }

    sql += QString::fromUtf8(";");
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

QVariantList CoreDB::getImagesFields(qlonglong imageID, DatabaseFields::Images fields)
{
    QVariantList values;

    if (fields != DatabaseFields::ImagesNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames = imagesFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM Images WHERE id=?;");

        d->db->execSql(query, imageID, &values);

        // Convert date times to QDateTime, they come as QString
        if ((fields & DatabaseFields::ModificationDate) && !values.isEmpty())
        {
            int index     = fieldNames.indexOf(QLatin1String("modificationDate"));
            values[index] = (values.at(index).isNull() ? QDateTime()
                             : QDateTime::fromString(values.at(index).toString(), Qt::ISODate));
        }
    }

    return values;
}

QVariantList CoreDB::getImageInformation(qlonglong imageID, DatabaseFields::ImageInformation fields)
{
    QVariantList values;

    if (fields != DatabaseFields::ImageInformationNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames = imageInformationFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM ImageInformation WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // Convert date times to QDateTime, they come as QString
        if ((fields & DatabaseFields::CreationDate) && !values.isEmpty())
        {
            int index     = fieldNames.indexOf(QLatin1String("creationDate"));
            values[index] = (values.at(index).isNull() ? QDateTime()
                                                       : QDateTime::fromString(values.at(index).toString(), Qt::ISODate));
        }

        if ((fields & DatabaseFields::DigitizationDate) && !values.isEmpty())
        {
            int index     = fieldNames.indexOf(QLatin1String("digitizationDate"));
            values[index] = (values.at(index).isNull() ? QDateTime()
                                                       : QDateTime::fromString(values.at(index).toString(), Qt::ISODate));
        }
    }

    return values;
}

QVariantList CoreDB::getImageMetadata(qlonglong imageID, DatabaseFields::ImageMetadata fields)
{
    QVariantList values;

    if (fields != DatabaseFields::ImageMetadataNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames = imageMetadataFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM ImageMetadata WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // For some reason, if REAL values may be required from variables stored as QString QVariants. Convert code will come here.
    }

    return values;
}

QVariantList CoreDB::getVideoMetadata(qlonglong imageID, DatabaseFields::VideoMetadata fields)
{
    QVariantList values;

    if (fields != DatabaseFields::VideoMetadataNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames = videoMetadataFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM VideoMetadata WHERE imageid=?;");

        d->db->execSql(query, imageID, &values);

        // For some reason REAL values may come as QString QVariants. Convert here.
        if (values.size() == fieldNames.size()        &&
            ((fields & DatabaseFields::Aperture)      ||
             (fields & DatabaseFields::FocalLength)   ||
             (fields & DatabaseFields::FocalLength35) ||
             (fields & DatabaseFields::ExposureTime)  ||
             (fields & DatabaseFields::SubjectDistance))
           )
        {
            for (int i = 0; i < values.size(); ++i)
            {
                if (values.at(i).type() == QVariant::String             &&
                    (fieldNames.at(i) == QLatin1String("aperture")      ||
                     fieldNames.at(i) == QLatin1String("focalLength")   ||
                     fieldNames.at(i) == QLatin1String("focalLength35") ||
                     fieldNames.at(i) == QLatin1String("exposureTime")  ||
                     fieldNames.at(i) == QLatin1String("subjectDistance"))
                   )
                {
                    values[i] = values.at(i).toDouble();
                }
            }
        }
    }

    return values;
}

QVariantList CoreDB::getImagePosition(qlonglong imageID, DatabaseFields::ImagePositions fields)
{
    QVariantList values;

    if (fields != DatabaseFields::ImagePositionsNone)
    {
        QString query(QString::fromUtf8("SELECT "));
        QStringList fieldNames =  imagePositionsFieldList(fields);
        query                 += fieldNames.join(QString::fromUtf8(", "));
        query                 += QString::fromUtf8(" FROM ImagePositions WHERE imageid=?;");

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
                    (fieldNames.at(i) == QLatin1String("latitudeNumber")  ||
                     fieldNames.at(i) == QLatin1String("longitudeNumber") ||
                     fieldNames.at(i) == QLatin1String("altitude")        ||
                     fieldNames.at(i) == QLatin1String("orientation")     ||
                     fieldNames.at(i) == QLatin1String("tilt")            ||
                     fieldNames.at(i) == QLatin1String("roll")            ||
                     fieldNames.at(i) == QLatin1String("accuracy"))
                   )
                {
                    if (!values.at(i).isNull())
                        values[i] = values.at(i).toDouble();
                }
            }
        }
    }

    return values;
}

QVariantList CoreDB::getImagePositions(QList<qlonglong> imageIDs, DatabaseFields::ImagePositions fields)
{
    QVariantList values;

    if (fields != DatabaseFields::ImagePositionsNone)
    {
        QString sql(QString::fromUtf8("SELECT "));
        QStringList fieldNames =  imagePositionsFieldList(fields);
        sql                    += fieldNames.join(QString::fromUtf8(", "));
        sql                    += QString::fromUtf8(" FROM ImagePositions WHERE imageid=?;");

        DbEngineSqlQuery query = d->db->prepareQuery(sql);

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
                    (fieldNames.at(i) == QLatin1String("latitudeNumber")  ||
                     fieldNames.at(i) == QLatin1String("longitudeNumber") ||
                     fieldNames.at(i) == QLatin1String("altitude")        ||
                     fieldNames.at(i) == QLatin1String("orientation")     ||
                     fieldNames.at(i) == QLatin1String("tilt")            ||
                     fieldNames.at(i) == QLatin1String("roll")            ||
                     fieldNames.at(i) == QLatin1String("accuracy"))
                   )
                {
                    if (!values.at(i).isNull())
                        values[i] = values.at(i).toDouble();
                }
            }
        }
    }

    return values;
}

void CoreDB::addImageInformation(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageInformation fields)
{
    if (fields == DatabaseFields::ImageInformationNone)
    {
        return;
    }

    QString query(QString::fromUtf8("REPLACE INTO ImageInformation ( imageid, "));

    QStringList fieldNames = imageInformationFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QLatin1String(", "));
    query += QString::fromUtf8(" ) VALUES (");
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += QString::fromUtf8(");");

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

void CoreDB::changeImageInformation(qlonglong imageId, const QVariantList& infos,
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

    d->db->execUpsertDBAction(QLatin1String("changeImageInformation"), imageId, fieldNames, checkedValues);

    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void CoreDB::addImageMetadata(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageMetadata fields)
{
    if (fields == DatabaseFields::ImageMetadataNone)
    {
        return;
    }

    QString query(QString::fromUtf8("REPLACE INTO ImageMetadata ( imageid, "));
    QStringList fieldNames = imageMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QLatin1String(", "));
    query += QString::fromUtf8(" ) VALUES (");
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += QString::fromUtf8(");");

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void CoreDB::changeImageMetadata(qlonglong imageId, const QVariantList& infos,
                                  DatabaseFields::ImageMetadata fields)
{
    if (fields == DatabaseFields::ImageMetadataNone)
    {
        return;
    }

    QString query(QString::fromUtf8("UPDATE ImageMetadata SET "));

    QStringList fieldNames = imageMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QString::fromUtf8("=?,"));
    query += QString::fromUtf8("=? WHERE imageid=?;");

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void CoreDB::addVideoMetadata(qlonglong imageID, const QVariantList& infos, DatabaseFields::VideoMetadata fields)
{
    if (fields == DatabaseFields::VideoMetadataNone)
    {
        return;
    }

    QString query(QString::fromUtf8("REPLACE INTO VideoMetadata ( imageid, ")); // need to create this database
    QStringList fieldNames = videoMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QLatin1String(", "));
    query += QString::fromUtf8(" ) VALUES (");
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += QString::fromUtf8(");");

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void CoreDB::changeVideoMetadata(qlonglong imageId, const QVariantList& infos,
                                  DatabaseFields::VideoMetadata fields)
{
    if (fields == DatabaseFields::VideoMetadataNone)
    {
        return;
    }

    QString query(QString::fromUtf8("UPDATE VideoMetadata SET "));
    QStringList fieldNames = videoMetadataFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QString::fromUtf8("=?,"));
    query += QString::fromUtf8("=? WHERE imageid=?;");

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void CoreDB::addImagePosition(qlonglong imageID, const QVariantList& infos, DatabaseFields::ImagePositions fields)
{
    if (fields == DatabaseFields::ImagePositionsNone)
    {
        return;
    }

    QString query(QString::fromUtf8("REPLACE INTO ImagePositions ( imageid, "));
    QStringList fieldNames = imagePositionsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QLatin1String(", "));
    query += QString::fromUtf8(" ) VALUES (");
    addBoundValuePlaceholders(query, infos.size() + 1);
    query += QString::fromUtf8(");");

    QVariantList boundValues;
    boundValues << imageID << infos;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void CoreDB::changeImagePosition(qlonglong imageId, const QVariantList& infos,
                                  DatabaseFields::ImagePositions fields)
{
    if (fields == DatabaseFields::ImagePositionsNone)
    {
        return;
    }

    QString query(QString::fromUtf8("UPDATE ImagePositions SET "));
    QStringList fieldNames = imagePositionsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QString::fromUtf8("=?,"));
    query += QString::fromUtf8("=? WHERE imageid=?;");

    QVariantList boundValues;
    boundValues << infos << imageId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageId, fields));
}

void CoreDB::removeImagePosition(qlonglong imageid)
{
    d->db->execSql(QString(QString::fromUtf8("DELETE FROM ImagePositions WHERE imageid=?;")),
                   imageid);

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::ImagePositionsAll));
}

void CoreDB::removeImagePositionAltitude(qlonglong imageid)
{
    d->db->execSql(QString(QString::fromUtf8("UPDATE ImagePositions SET altitude=NULL WHERE imageid=?;")),
                   imageid);

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::Altitude));
}

QList<CommentInfo> CoreDB::getImageComments(qlonglong imageID)
{
    QList<CommentInfo> list;

    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id, type, language, author, date, comment "
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

int CoreDB::setImageComment(qlonglong imageID, const QString& comment, DatabaseComment::Type type,
                             const QString& language, const QString& author, const QDateTime& date)
{
    QVariantList boundValues;
    boundValues << imageID << (int)type << language << author << date << comment;

    QVariant id;
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageComments "
                           "( imageid, type, language, author, date, comment ) "
                           " VALUES (?,?,?,?,?,?);"),
                   boundValues, 0, &id);

    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::ImageCommentsAll));
    return id.toInt();
}

void CoreDB::changeImageComment(int commentId, qlonglong imageID, const QVariantList& infos, DatabaseFields::ImageComments fields)
{
    if (fields == DatabaseFields::ImageCommentsNone)
    {
        return;
    }

    QString query(QString::fromUtf8("UPDATE ImageComments SET "));
    QStringList fieldNames = imageCommentsFieldList(fields);

    Q_ASSERT(fieldNames.size() == infos.size());

    query += fieldNames.join(QString::fromUtf8("=?,"));
    query += QString::fromUtf8("=? WHERE id=?;");

    QVariantList boundValues;
    boundValues << infos << commentId;

    d->db->execSql(query, boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, fields));
}

void CoreDB::removeImageComment(int commentid, qlonglong imageid)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageComments WHERE id=?;"),
                   commentid);

    d->db->recordChangeset(ImageChangeset(imageid, DatabaseFields::ImageCommentsAll));
}

QString CoreDB::getImageProperty(qlonglong imageID, const QString& property)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT value FROM ImageProperties "
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

void CoreDB::setImageProperty(qlonglong imageID, const QString& property, const QString& value)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageProperties "
                           "(imageid, property, value) "
                           "VALUES(?, ?, ?);"),
                   imageID, property, value);
}

void CoreDB::removeImageProperty(qlonglong imageID, const QString& property)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageProperties WHERE imageid=? AND property=?;"),
                   imageID, property);
}

void CoreDB::removeImagePropertyByName(const QString& property)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageProperties WHERE property=?;"),
                   property);
}

QList<CopyrightInfo> CoreDB::getImageCopyright(qlonglong imageID, const QString& property)
{
    QList<CopyrightInfo> list;
    QList<QVariant>      values;

    if (property.isNull())
    {
        d->db->execSql(QString::fromUtf8("SELECT property, value, extraValue FROM ImageCopyright "
                               "WHERE imageid=?;"),
                       imageID, &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT property, value, extraValue FROM ImageCopyright "
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

void CoreDB::setImageCopyrightProperty(qlonglong imageID, const QString& property,
                                        const QString& value, const QString& extraValue,
                                        CopyrightPropertyUnique uniqueness)
{
    if (uniqueness == PropertyUnique)
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                               "WHERE imageid=? AND property=?;"),
                       imageID, property);
    }
    else if (uniqueness == PropertyExtraValueUnique)
    {
        d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                               "WHERE imageid=? AND property=? AND extraValue=?;"),
                       imageID, property, extraValue);
    }

    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageCopyright "
                           "(imageid, property, value, extraValue) "
                           "VALUES(?, ?, ?, ?);"),
                   imageID, property, value, extraValue);
}

void CoreDB::removeImageCopyrightProperties(qlonglong imageID, const QString& property,
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
            d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                   "WHERE imageid=?;"),
                           imageID);
            break;

        case 1:
            d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                   "WHERE imageid=? AND property=?;"),
                           imageID, property);
            break;

        case 2:
            d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                   "WHERE imageid=? AND property=? AND extraValue=?;"),
                           imageID, property, extraValue);
            break;

        case 3:
            d->db->execSql(QString::fromUtf8("DELETE FROM ImageCopyright "
                                   "WHERE imageid=? AND property=? AND extraValue=? AND value=?;"),
                           imageID, property, extraValue, value);
            break;
    }
}

QList<qlonglong> CoreDB::findByNameAndCreationDate(const QString& fileName, const QDateTime& creationDate)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images "
                   " LEFT JOIN ImageInformation ON id=imageid "
                   "WHERE name=? AND creationDate=? AND status!=3;"),
                   fileName, creationDate.toString(Qt::ISODate), &values);

    QList<qlonglong> ids;

    foreach(const QVariant& var, values)
    {
        ids << var.toLongLong();
    }

    return ids;
}

bool CoreDB::hasImageHistory(qlonglong imageId)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT history FROM ImageHistory WHERE imageid=?;"),
                   imageId, &values);

    return !values.isEmpty();
}

ImageHistoryEntry CoreDB::getImageHistory(qlonglong imageId)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT uuid, history FROM ImageHistory WHERE imageid=?;"),
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

QList<qlonglong> CoreDB::getItemsForUuid(const QString& uuid)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT imageid FROM ImageHistory "
                   "INNER JOIN Images ON imageid=id "
                   "WHERE uuid=? AND status!=3;"),
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

QString CoreDB::getImageUuid(qlonglong imageId)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT uuid FROM ImageHistory WHERE imageid=?;"),
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

void CoreDB::setImageHistory(qlonglong imageId, const QString& history)
{
    d->db->execUpsertDBAction(QLatin1String("changeImageHistory"), imageId, QStringList() << QLatin1String("history"), QVariantList() << history);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::ImageHistory));
}

void CoreDB::setImageUuid(qlonglong imageId, const QString& uuid)
{
    d->db->execUpsertDBAction(QLatin1String("changeImageHistory"), imageId, QStringList() << QLatin1String("uuid"), QVariantList() << uuid);
    d->db->recordChangeset(ImageChangeset(imageId, DatabaseFields::ImageUUID));
}

void CoreDB::addImageRelation(qlonglong subjectId, qlonglong objectId, DatabaseRelation::Type type)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageRelations (subject, object, type) VALUES (?, ?, ?);"),
                   subjectId, objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << subjectId << objectId, DatabaseFields::ImageRelations));
}

void CoreDB::addImageRelations(const QList<qlonglong>& subjectIds, const QList<qlonglong>& objectIds, DatabaseRelation::Type type)
{
    DbEngineSqlQuery query = d->db->prepareQuery(QString::fromUtf8("REPLACE INTO ImageRelations (subject, object, type) VALUES (?, ?, ?);"));

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


void CoreDB::addImageRelation(const ImageRelation& relation)
{
    addImageRelation(relation.subjectId, relation.objectId, relation.type);
}

void CoreDB::removeImageRelation(qlonglong subjectId, qlonglong objectId, DatabaseRelation::Type type)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageRelations WHERE subject=? AND object=? AND type=?;"),
                   subjectId, objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << subjectId << objectId, DatabaseFields::ImageRelations));
}

void CoreDB::removeImageRelation(const ImageRelation& relation)
{
    removeImageRelation(relation.subjectId, relation.objectId, relation.type);
}

QList<qlonglong> CoreDB::removeAllImageRelationsTo(qlonglong objectId, DatabaseRelation::Type type)
{
    QList<qlonglong> affected = getImagesRelatingTo(objectId, type);

    if (affected.isEmpty())
    {
        return affected;
    }

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageRelations WHERE object=? AND type=?;"),
                   objectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << affected << objectId, DatabaseFields::ImageRelations));

    return affected;
}

QList<qlonglong> CoreDB::removeAllImageRelationsFrom(qlonglong subjectId, DatabaseRelation::Type type)
{
    QList<qlonglong> affected = getImagesRelatedFrom(subjectId, type);

    if (affected.isEmpty())
    {
        return affected;
    }

    d->db->execSql(QString::fromUtf8("DELETE FROM ImageRelations WHERE subject=? AND type=?;"),
                   subjectId, type);
    d->db->recordChangeset(ImageChangeset(QList<qlonglong>() << affected << subjectId, DatabaseFields::ImageRelations));

    return affected;
}

QList<qlonglong> CoreDB::getImagesRelatedFrom(qlonglong subjectId, DatabaseRelation::Type type)
{
    return getRelatedImages(subjectId, true, type, false);
}

QVector<QList<qlonglong> > CoreDB::getImagesRelatedFrom(QList<qlonglong> subjectIds, DatabaseRelation::Type type)
{
    return getRelatedImages(subjectIds, true, type, false);
}

bool CoreDB::hasImagesRelatedFrom(qlonglong subjectId, DatabaseRelation::Type type)
{
    // returns 0 or 1 item in list
    return !getRelatedImages(subjectId, true, type, true).isEmpty();
}

QList<qlonglong> CoreDB::getImagesRelatingTo(qlonglong objectId, DatabaseRelation::Type type)
{
    return getRelatedImages(objectId, false, type, false);
}

QVector<QList<qlonglong> > CoreDB::getImagesRelatingTo(QList<qlonglong> objectIds, DatabaseRelation::Type type)
{
    return getRelatedImages(objectIds, false, type, false);
}

bool CoreDB::hasImagesRelatingTo(qlonglong objectId, DatabaseRelation::Type type)
{
    // returns 0 or 1 item in list
    return !getRelatedImages(objectId, false, type, true).isEmpty();
}

QList<qlonglong> CoreDB::getRelatedImages(qlonglong id, bool fromOrTo, DatabaseRelation::Type type, bool boolean)
{
    QString sql = d->constructRelatedImagesSQL(fromOrTo, type, boolean);
    DbEngineSqlQuery query = d->db->prepareQuery(sql);
    return d->execRelatedImagesQuery(query, id, type);
}

QVector<QList<qlonglong> > CoreDB::getRelatedImages(QList<qlonglong> ids,
                                                     bool fromOrTo, DatabaseRelation::Type type, bool boolean)
{
    if (ids.isEmpty())
    {
        return QVector<QList<qlonglong> >();
    }

    QVector<QList<qlonglong> > result(ids.size());

    QString sql = d->constructRelatedImagesSQL(fromOrTo, type, boolean);
    DbEngineSqlQuery query = d->db->prepareQuery(sql);

    for (int i = 0; i < ids.size(); i++)
    {
        result[i] = d->execRelatedImagesQuery(query, ids[i], type);
    }

    return result;
}

QList<QPair<qlonglong, qlonglong> > CoreDB::getRelationCloud(qlonglong imageId, DatabaseRelation::Type type)
{
    QSet<qlonglong> todo, done;
    QSet<QPair<qlonglong, qlonglong> > pairs;
    todo << imageId;

    QString sql = QString::fromUtf8(
                  "SELECT subject, object FROM ImageRelations "
                  "INNER JOIN Images AS SubjectImages ON ImageRelations.subject=SubjectImages.id "
                  "INNER JOIN Images AS ObjectImages  ON ImageRelations.object=ObjectImages.id "
                  "WHERE (subject=? OR object=?) %1 AND SubjectImages.status!=3 AND ObjectImages.status!=3;");

    if (type == DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString());
    }
    else
    {
        sql = sql.arg(QString::fromUtf8("AND type=?"));
    }

    DbEngineSqlQuery query = d->db->prepareQuery(sql);

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

QList<qlonglong> CoreDB::getOneRelatedImageEach(const QList<qlonglong>& ids, DatabaseRelation::Type type)
{
    QString sql = QString::fromUtf8(
                  "SELECT subject, object FROM ImageRelations "
                  "INNER JOIN Images AS SubjectImages ON ImageRelations.subject=SubjectImages.id "
                  "INNER JOIN Images AS ObjectImages  ON ImageRelations.object=ObjectImages.id "
                  "WHERE ( (subject=? AND ObjectImages.status!=3) "
                  "     OR (object=? AND SubjectImages.status!=3) ) "
                  " %1 LIMIT 1;");

    if (type == DatabaseRelation::UndefinedType)
    {
        sql = sql.arg(QString());
    }
    else
    {
        sql = sql.arg(QString::fromUtf8("AND type=?"));
    }

    DbEngineSqlQuery        query = d->db->prepareQuery(sql);
    QSet<qlonglong> result;
    QList<QVariant> values;

    foreach(const qlonglong& id, ids)
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

QStringList CoreDB::getItemsURLsWithTag(int tagId)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
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

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
        }
    }

    return urls;
}

QStringList CoreDB::getDirtyOrMissingFaceImageUrls()
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT Albums.albumRoot, Albums.relativePath, Images.name FROM Images "
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

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
        }
    }

    return urls;
}

QList<ItemScanInfo> CoreDB::getIdenticalFiles(qlonglong id)
{
    if (!id)
    {
        return QList<ItemScanInfo>();
    }

    QList<QVariant> values;

    // retrieve unique hash and file size
    d->db->execSql(QString::fromUtf8("SELECT uniqueHash, fileSize FROM Images WHERE id=?; "),
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

QList<ItemScanInfo> CoreDB::getIdenticalFiles(const QString& uniqueHash, qlonglong fileSize, qlonglong sourceId)
{
    // enforce validity
    if (uniqueHash.isEmpty() || fileSize <= 0)
    {
        return QList<ItemScanInfo>();
    }

    QList<QVariant> values;

    // find items with same fingerprint
    d->db->execSql(QString::fromUtf8("SELECT id, album, name, status, category, modificationDate, fileSize FROM Images "
                           " WHERE fileSize=? AND uniqueHash=? AND album IS NOT NULL; "),
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

QStringList CoreDB::imagesFieldList(DatabaseFields::Images fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::Album)
    {
        list << QLatin1String("album");
    }

    if (fields & DatabaseFields::Name)
    {
        list << QLatin1String("name");
    }

    if (fields & DatabaseFields::Status)
    {
        list << QLatin1String("status");
    }

    if (fields & DatabaseFields::Category)
    {
        list << QLatin1String("category");
    }

    if (fields & DatabaseFields::ModificationDate)
    {
        list << QLatin1String("modificationDate");
    }

    if (fields & DatabaseFields::FileSize)
    {
        list << QLatin1String("fileSize");
    }

    if (fields & DatabaseFields::UniqueHash)
    {
        list << QLatin1String("uniqueHash");
    }

    return list;
}

QStringList CoreDB::imageInformationFieldList(DatabaseFields::ImageInformation fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::Rating)
    {
        list << QLatin1String("rating");
    }

    if (fields & DatabaseFields::CreationDate)
    {
        list << QLatin1String("creationDate");
    }

    if (fields & DatabaseFields::DigitizationDate)
    {
        list << QLatin1String("digitizationDate");
    }

    if (fields & DatabaseFields::Orientation)
    {
        list << QLatin1String("orientation");
    }

    if (fields & DatabaseFields::Width)
    {
        list << QLatin1String("width");
    }

    if (fields & DatabaseFields::Height)
    {
        list << QLatin1String("height");
    }

    if (fields & DatabaseFields::Format)
    {
        list << QLatin1String("format");
    }

    if (fields & DatabaseFields::ColorDepth)
    {
        list << QLatin1String("colorDepth");
    }

    if (fields & DatabaseFields::ColorModel)
    {
        list << QLatin1String("colorModel");
    }

    return list;
}

QStringList CoreDB::videoMetadataFieldList(DatabaseFields::VideoMetadata fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::AspectRatio)
    {
        list << QLatin1String("aspectRatio");
    }

    if (fields & DatabaseFields::AudioBitRate)
    {
        list << QLatin1String("audioBitRate");
    }

    if (fields & DatabaseFields::AudioChannelType)
    {
        list << QLatin1String("audioChannelType");
    }

    if (fields & DatabaseFields::AudioCodec)
    {
        list << QLatin1String("audioCompressor");
    }

    if (fields & DatabaseFields::Duration)
    {
        list << QLatin1String("duration");
    }

    if (fields & DatabaseFields::FrameRate)
    {
        list << QLatin1String("frameRate");
    }

    if (fields & DatabaseFields::VideoCodec)
    {
        list << QLatin1String("videoCodec");
    }

    return list;
}

QStringList CoreDB::imageMetadataFieldList(DatabaseFields::ImageMetadata fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::Make)
    {
        list << QLatin1String("make");
    }

    if (fields & DatabaseFields::Model)
    {
        list << QLatin1String("model");
    }

    if (fields & DatabaseFields::Lens)
    {
        list << QLatin1String("lens");
    }

    if (fields & DatabaseFields::Aperture)
    {
        list << QLatin1String("aperture");
    }

    if (fields & DatabaseFields::FocalLength)
    {
        list << QLatin1String("focalLength");
    }

    if (fields & DatabaseFields::FocalLength35)
    {
        list << QLatin1String("focalLength35");
    }

    if (fields & DatabaseFields::ExposureTime)
    {
        list << QLatin1String("exposureTime");
    }

    if (fields & DatabaseFields::ExposureProgram)
    {
        list << QLatin1String("exposureProgram");
    }

    if (fields & DatabaseFields::ExposureMode)
    {
        list << QLatin1String("exposureMode");
    }

    if (fields & DatabaseFields::Sensitivity)
    {
        list << QLatin1String("sensitivity");
    }

    if (fields & DatabaseFields::FlashMode)
    {
        list << QLatin1String("flash");
    }

    if (fields & DatabaseFields::WhiteBalance)
    {
        list << QLatin1String("whiteBalance");
    }

    if (fields & DatabaseFields::WhiteBalanceColorTemperature)
    {
        list << QLatin1String("whiteBalanceColorTemperature");
    }

    if (fields & DatabaseFields::MeteringMode)
    {
        list << QLatin1String("meteringMode");
    }

    if (fields & DatabaseFields::SubjectDistance)
    {
        list << QLatin1String("subjectDistance");
    }

    if (fields & DatabaseFields::SubjectDistanceCategory)
    {
        list << QLatin1String("subjectDistanceCategory");
    }

    return list;
}

QStringList CoreDB::imagePositionsFieldList(DatabaseFields::ImagePositions fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::Latitude)
    {
        list << QLatin1String("latitude");
    }

    if (fields & DatabaseFields::LatitudeNumber)
    {
        list << QLatin1String("latitudeNumber");
    }

    if (fields & DatabaseFields::Longitude)
    {
        list << QLatin1String("longitude");
    }

    if (fields & DatabaseFields::LongitudeNumber)
    {
        list << QLatin1String("longitudeNumber");
    }

    if (fields & DatabaseFields::Altitude)
    {
        list << QLatin1String("altitude");
    }

    if (fields & DatabaseFields::PositionOrientation)
    {
        list << QLatin1String("orientation");
    }

    if (fields & DatabaseFields::PositionTilt)
    {
        list << QLatin1String("tilt");
    }

    if (fields & DatabaseFields::PositionRoll)
    {
        list << QLatin1String("roll");
    }

    if (fields & DatabaseFields::PositionAccuracy)
    {
        list << QLatin1String("accuracy");
    }

    if (fields & DatabaseFields::PositionDescription)
    {
        list << QLatin1String("description");
    }

    return list;
}

QStringList CoreDB::imageCommentsFieldList(DatabaseFields::ImageComments fields)
{
    // adds no spaces at beginning or end
    QStringList list;

    if (fields & DatabaseFields::CommentType)
    {
        list << QLatin1String("type");
    }

    if (fields & DatabaseFields::CommentLanguage)
    {
        list << QLatin1String("language");
    }

    if (fields & DatabaseFields::CommentAuthor)
    {
        list << QLatin1String("author");
    }

    if (fields & DatabaseFields::CommentDate)
    {
        list << QLatin1String("date");
    }

    if (fields & DatabaseFields::Comment)
    {
        list << QLatin1String("comment");
    }

    return list;
}

void CoreDB::addBoundValuePlaceholders(QString& query, int count)
{
    // adds no spaces at beginning or end
    QString questionMarks;
    questionMarks.reserve(count * 2);
    QString questionMark(QString::fromUtf8("?,"));

    for (int i = 0; i < count; ++i)
    {
        questionMarks += questionMark;
    }

    // remove last ','
    questionMarks.chop(1);

    query += questionMarks;
}

int CoreDB::findInDownloadHistory(const QString& identifier, const QString& name, qlonglong fileSize, const QDateTime& date)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id FROM DownloadHistory WHERE "
                                     "identifier=? AND filename=? AND filesize=? AND filedate=?;"),
                   identifier, name, fileSize, date.toString(Qt::ISODate), &values);

    if (values.isEmpty())
    {
        return -1;
    }

    return values.first().toInt();
}

int CoreDB::addToDownloadHistory(const QString& identifier, const QString& name, qlonglong fileSize, const QDateTime& date)
{
    QVariant id;
    d->db->execSql(QString::fromUtf8("REPLACE INTO DownloadHistory "
                           "(identifier, filename, filesize, filedate) "
                           "VALUES (?,?,?,?);"),
                   identifier, name, fileSize, date.toString(Qt::ISODate), 0, &id);

    return id.toInt();
}

/*
void CoreDB::setItemCaption(qlonglong imageID,const QString& caption)
{
    QList<QVariant> values;

    d->db->execSql( QString::fromUtf8("UPDATE Images SET caption=? "
                            "WHERE id=?;"),
                    caption,
                    imageID );

    CoreDbAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageComment);
}


void CoreDB::setItemCaption(int albumID, const QString& name, const QString& caption)
{
    / *
    QList<QVariant> values;

    d->db->execSql( QString::fromUtf8("UPDATE Images SET caption=? "
                     "WHERE dirid=? AND name=?;")
             .(caption,
                  QString::number(albumID),
                  (name)) );
    * /

    // easier because of attributes watch
    return setItemCaption(getImageId(albumID, name), caption);
}
*/

void CoreDB::addItemTag(qlonglong imageID, int tagID)
{
    d->db->execSql(QString::fromUtf8("REPLACE INTO ImageTags (imageid, tagid) "
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

void CoreDB::addItemTag(int albumID, const QString& name, int tagID)
{
/*
    d->db->execSql( QString::fromUtf8("REPLACE INTO ImageTags (imageid, tagid) \n "
                     "(SELECT id, ? FROM Images \n "
                     " WHERE dirid=? AND name=?);")
             .tagID
             .albumID
             .(name) );
*/

    // easier because of attributes watch
    return addItemTag(getImageId(albumID, name), tagID);
}

void CoreDB::addTagsToItems(QList<qlonglong> imageIDs, QList<int> tagIDs)
{
    if (imageIDs.isEmpty() || tagIDs.isEmpty())
    {
        return;
    }

    DbEngineSqlQuery     query = d->db->prepareQuery(QString::fromUtf8("REPLACE INTO ImageTags (imageid, tagid) VALUES(?, ?);"));
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

QList<int> CoreDB::getRecentlyAssignedTags() const
{
    return d->recentlyAssignedTags;
}

void CoreDB::removeItemTag(qlonglong imageID, int tagID)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageTags "
                           "WHERE imageID=? AND tagid=?;"),
                   imageID,
                   tagID);

    d->db->recordChangeset(ImageTagChangeset(imageID, tagID, ImageTagChangeset::Removed));
}

void CoreDB::removeItemAllTags(qlonglong imageID, const QList<int>& currentTagIds)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM ImageTags "
                           "WHERE imageID=?;"),
                   imageID);

    d->db->recordChangeset(ImageTagChangeset(imageID, currentTagIds, ImageTagChangeset::RemovedAll));
}

void CoreDB::removeTagsFromItems(QList<qlonglong> imageIDs, const QList<int>& tagIDs)
{
    if (imageIDs.isEmpty() || tagIDs.isEmpty())
    {
        return;
    }

    DbEngineSqlQuery     query = d->db->prepareQuery(QString::fromUtf8("DELETE FROM ImageTags WHERE imageID=? AND tagid=?;"));
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

QStringList CoreDB::getItemNamesInAlbum(int albumID, bool recursive)
{
    QList<QVariant> values;

    if (recursive)
    {
        int rootId = getAlbumRootId(albumID);
        QString path = getAlbumRelativePath(albumID);
        d->db->execSql(QString::fromUtf8("SELECT Images.name FROM Images WHERE Images.album IN "
                               " (SELECT DISTINCT id FROM Albums "
                               "  WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?));"),
                       rootId, path, path == QString::fromUtf8("/") ? QString::fromUtf8("/%")
                                                                    : QString(path + QLatin1String("/%")),
                       &values);
    }
    else
    {
        d->db->execSql(QString::fromUtf8("SELECT Images.name "
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

qlonglong CoreDB::getItemFromAlbum(int albumID, const QString& fileName)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT Images.id "
                               "FROM Images "
                               "WHERE Images.album=? AND Images.name=?"),
                       albumID,fileName, &values);

    if (values.isEmpty())
    {
        return -1;
    }
    else
    {
        return values.first().toLongLong();
    }
}

/*
QStringList CoreDB::getAllItemURLsWithoutDate()
{
    QList<QVariant> values;
    d->db->execSql( QString::fromUtf8("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
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

QList<QDateTime> CoreDB::getAllCreationDates()
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT creationDate FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.status=1;"), &values);

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

QMap<QDateTime, int> CoreDB::getAllCreationDatesAndNumberOfImages()
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT creationDate FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.status=1;"), &values);

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

QMap<int, int> CoreDB::getNumberOfImagesInAlbums()
{
    QList<QVariant> values, allAbumIDs;
    QMap<int, int>  albumsStatMap;
    int             albumID;

    // initialize allAbumIDs with all existing albums from db to prevent
    // wrong album image counters
    d->db->execSql(QString::fromUtf8("SELECT id from Albums"), &allAbumIDs);

    for (QList<QVariant>::const_iterator it = allAbumIDs.constBegin(); it != allAbumIDs.constEnd(); ++it)
    {
        albumID = (*it).toInt();
        albumsStatMap.insert(albumID, 0);
    }

    d->db->execSql(QString::fromUtf8("SELECT album FROM Images WHERE Images.status=1;"), &values);

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

QMap<int, int> CoreDB::getNumberOfImagesInTags()
{
    QList<QVariant> values, allTagIDs;
    QMap<int, int>  tagsStatMap;
    int             tagID;

    // initialize allTagIDs with all existing tags from db to prevent
    // wrong tag counters
    d->db->execSql(QString::fromUtf8("SELECT id from Tags"), &allTagIDs);

    for (QList<QVariant>::const_iterator it = allTagIDs.constBegin(); it != allTagIDs.constEnd(); ++it)
    {
        tagID = (*it).toInt();
        tagsStatMap.insert(tagID, 0);
    }

    d->db->execSql(QString::fromUtf8("SELECT tagid FROM ImageTags "
                   " LEFT JOIN Images ON Images.id=ImageTags.imageid "
                   " WHERE Images.status=1;"), &values);

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

QMap<int, int> CoreDB::getNumberOfImagesInTagProperties(const QString& property)
{
    QList<QVariant> values;
    QMap<int, int>  tagsStatMap;
    int             tagID, count;

    d->db->execSql(QString::fromUtf8("SELECT tagid, COUNT(*) FROM ImageTagProperties "
                   " LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                   " WHERE ImageTagProperties.property=? AND Images.status=1 "
                   " GROUP BY tagid;"),
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

int CoreDB::getNumberOfImagesInTagProperties(int tagId, const QString& property)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT COUNT(*) FROM ImageTagProperties "
                   " LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                   " WHERE ImageTagProperties.property=? AND Images.status=1 "
                   " AND ImageTagProperties.tagid=? ;"),
                   property, tagId, &values);

    return values.first().toInt();
}

QList<qlonglong> CoreDB::getImagesWithImageTagProperty(int tagId, const QString& property)
{
    QList<QVariant> values;
    QList<qlonglong> imageIds;
    d->db->execSql(QString::fromUtf8("SELECT DISTINCT Images.id FROM ImageTagProperties "
                   " LEFT JOIN Images ON Images.id=ImageTagProperties.imageid "
                   " WHERE ImageTagProperties.property=? AND Images.status=1 "
                   " AND ImageTagProperties.tagid=? ;"),
                   property, tagId, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        imageIds.append((*it).toInt());
    }

    return imageIds;
}

QMap<QString, int> CoreDB::getFormatStatistics()
{
    return getFormatStatistics(DatabaseItem::UndefinedCategory);
}

QMap<QString, int> CoreDB::getFormatStatistics(DatabaseItem::Category category)
{
    QMap<QString, int>  map;

    QString queryString = QString::fromUtf8(
                          "SELECT COUNT(*), II.format "
                          "  FROM ImageInformation AS II "
                          "  INNER JOIN Images ON II.imageid=Images.id "
                          "  WHERE Images.status=1 ");

    if (category != DatabaseItem::UndefinedCategory)
    {
        queryString.append(QString::fromUtf8("AND Images.category=%1").arg(category));
    }

    queryString.append(QString::fromUtf8(" GROUP BY II.format;"));
    qCDebug(DIGIKAM_DATABASE_LOG) << queryString;

    DbEngineSqlQuery query = d->db->prepareQuery(queryString);

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

QStringList CoreDB::getListFromImageMetadata(DatabaseFields::ImageMetadata field)
{
    QStringList list;
    QList<QVariant> values;
    QStringList fieldName = imageMetadataFieldList(field);

    if (fieldName.count() != 1)
    {
        return list;
    }

    QString sql = QString::fromUtf8("SELECT DISTINCT %1 FROM ImageMetadata "
                                    " INNER JOIN Images ON imageid=Images.id;");

    sql = sql.arg(fieldName.first());
    d->db->execSql(sql, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        if (!it->isNull())
        {
            list << it->toString();
        }
    }

    return list;
}

/*
QList<QPair<QString, QDateTime> > CoreDB::getItemsAndDate()
{
    QList<QVariant> values;
    d->db->execSql( QString::fromUtf8("SELECT Images.name, datetime FROM Images;", &values ));

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
int CoreDB::getAlbumForPath(const QString& albumRoot, const QString& folder, bool create)
{
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);
    if (location.isNull())
        return -1;

    return getAlbumForPath(location.id(), folder, create);

}
*/

int CoreDB::getAlbumForPath(int albumRootId, const QString& folder, bool create)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id FROM Albums WHERE albumRoot=? AND relativePath=?;"),
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

QList<int> CoreDB::getAlbumAndSubalbumsForPath(int albumRootId, const QString& relativePath)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id, relativePath FROM Albums WHERE albumRoot=? AND (relativePath=? OR relativePath LIKE ?);"),
                   albumRootId, relativePath, (relativePath == QLatin1String("/") ? QLatin1String("/%")
                                                                                  : QString(relativePath + QLatin1String("/%"))), &values);

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

QList<int> CoreDB::getAlbumsOnAlbumRoot(int albumRootId)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT id FROM Albums WHERE albumRoot=?;"),
                   albumRootId, &values);

    QList<int> albumIds;

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        albumIds << (*it).toInt();
    }

    return albumIds;
}

qlonglong CoreDB::addItem(int albumID, const QString& name,
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
    d->db->execSql(QString::fromUtf8("REPLACE INTO Images "
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

void CoreDB::updateItem(qlonglong imageID, DatabaseItem::Category category,
                         const QDateTime& modificationDate,
                         qlonglong fileSize, const QString& uniqueHash)
{
    QVariantList boundValues;
    boundValues << (int)category << modificationDate.toString(Qt::ISODate) << fileSize << uniqueHash << imageID;
    d->db->execSql(QString::fromUtf8("UPDATE Images SET category=?, modificationDate=?, fileSize=?, uniqueHash=? WHERE id=?;"),
                   boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Category
                                          | DatabaseFields::ModificationDate
                                          | DatabaseFields::FileSize
                                          | DatabaseFields::UniqueHash));
}

void CoreDB::setItemStatus(qlonglong imageID, DatabaseItem::Status status)
{
    QVariantList boundValues;
    boundValues << (int)status << imageID;
    d->db->execSql(QString::fromUtf8("UPDATE Images SET status=? WHERE id=?;"),
                   boundValues);
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Status));
}

void CoreDB::setItemAlbum(qlonglong imageID, qlonglong album)
{
    QVariantList boundValues;
    boundValues << album << imageID;
    d->db->execSql(QString::fromUtf8("UPDATE Images SET album=? WHERE id=?;"),
                   boundValues);

    // record that the image was assigned a new album
    d->db->recordChangeset(ImageChangeset(imageID, DatabaseFields::Album));
    // also record that the collection was changed by adding an image to an album.
    d->db->recordChangeset(CollectionImageChangeset(imageID, album, CollectionImageChangeset::Added));
}

/*
QList<int> CoreDB::getTagsFromTagPaths(const QStringList& keywordsList, bool create)
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

int CoreDB::getItemAlbum(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT album FROM Images WHERE id=?;"),
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

QString CoreDB::getItemName(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT name FROM Images WHERE id=?;"),
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
bool CoreDB::setItemDate(qlonglong imageID,
                          const QDateTime& datetime)
{
    d->db->execSql ( QString::fromUtf8("UPDATE Images SET datetime=?"
                            "WHERE id=?;"),
                     datetime.toString(Qt::ISODate),
                     imageID );

    CoreDbAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageDate);

    return true;
}

bool CoreDB::setItemDate(int albumID, const QString& name,
                          const QDateTime& datetime)
{
    / *
    d->db->execSql ( QString::fromUtf8("UPDATE Images SET datetime=?"
                       "WHERE dirid=? AND name=?;")
              .datetime.toString(Qt::ISODate,
                   QString::number(albumID),
                   (name)) );

    return true;
    * /
    // easier because of attributes watch
    return setItemDate(getImageId(albumID, name), datetime);
}


void CoreDB::setItemRating(qlonglong imageID, int rating)
{
    d->db->execSql ( QString::fromUtf8("REPLACE INTO ImageProperties "
                            "(imageid, property, value) "
                            "VALUES(?, ?, ?);"),
                     imageID,
                     QString("Rating"),
                     rating );

    CoreDbAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageRating);
}

int CoreDB::getItemRating(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString::fromUtf8(SELECT value FROM ImageProperties "
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

QStringList CoreDB::getItemURLsInAlbum(int albumID, ItemSortOrder sortOrder)
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
    bindingMap.insert(QString::fromUtf8(":albumID"), albumID);

    switch (sortOrder)
    {
        case ByItemName:
            d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("getItemURLsInAlbumByItemName")), bindingMap, &values);
            break;

        case ByItemPath:
            // Don't collate on the path - this is to maintain the same behavior
            // that happens when sort order is "By Path"
            d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("getItemURLsInAlbumByItemPath")), bindingMap, &values);
            break;

        case ByItemDate:
            d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("getItemURLsInAlbumByItemDate")), bindingMap, &values);
            break;

        case ByItemRating:
            d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("getItemURLsInAlbumByItemRating")), bindingMap, &values);
            break;

        case NoItemSorting:
        default:
            d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("getItemURLsInAlbumNoItemSorting")), bindingMap, &values);
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

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
        }
    }

    return urls;
}

QList<qlonglong> CoreDB::getItemIDsInAlbum(int albumID)
{
    QList<qlonglong> itemIDs;
    QList<QVariant>  values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images WHERE album=?;"),
                   albumID, &values);

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

QMap<qlonglong, QString> CoreDB::getItemIDsAndURLsInAlbum(int albumID)
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

    d->db->execSql(QString::fromUtf8("SELECT Images.id, Albums.relativePath, Images.name "
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

        if (relativePath == QLatin1String("/"))
        {
            path = albumRootPath + relativePath + name;
        }
        else
        {
            path = albumRootPath + relativePath + QLatin1Char('/') + name;
        }

        itemsMap.insert(id, path);
    };

    return itemsMap;
}

QList<qlonglong> CoreDB::getAllItems()
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT id FROM Images;"),
                    &values);

    QList<qlonglong> items;
    foreach(QVariant item, values)
    {
        items << item.toLongLong();
    }

    return items;
}

QList<ItemScanInfo> CoreDB::getItemScanInfos(int albumID)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT id, album, name, status, category, modificationDate, fileSize, uniqueHash "
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

ItemScanInfo CoreDB::getItemScanInfo(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT id, album, name, status, category, modificationDate, fileSize, uniqueHash "
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

QStringList CoreDB::getItemURLsInTag(int tagID, bool recursive)
{
    QList<QVariant>         values;
    QString                 imagesIdClause;
    QMap<QString, QVariant> bindingMap;

    bindingMap.insert(QString::fromUtf8(":tagID"), tagID);
    bindingMap.insert(QString::fromUtf8(":tagID2"), tagID);

    if (recursive)
    {
        d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("GetItemURLsInTagRecursive")), bindingMap, &values);
    }
    else
    {
        d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("GetItemURLsInTag")), bindingMap, &values);
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

        if (relativePath == QLatin1String("/"))
        {
            urls << albumRootPath + relativePath + name;
        }
        else
        {
            urls << albumRootPath + relativePath + QLatin1Char('/') + name;
        }
    }

    return urls;
}

QList<qlonglong> CoreDB::getItemIDsInTag(int tagID, bool recursive)
{
    QList<qlonglong>        itemIDs;
    QList<QVariant>         values;
    QMap<QString, QVariant> parameters;

    parameters.insert(QString::fromUtf8(":tagPID"), tagID);
    parameters.insert(QString::fromUtf8(":tagID"),  tagID);

    if (recursive)
    {
        d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("getItemIDsInTagRecursive")), parameters, &values);
    }
    else
    {
        d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("getItemIDsInTag")), parameters, &values);
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

/*
QString CoreDB::getAlbumPath(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString::fromUtf8("SELECT AlbumRoots.absolutePath||Albums.relativePath "
                            "FROM Albums, AlbumRoots WHERE Albums.id=? AND AlbumRoots.id=Albums.albumRoot; "),
                    albumID, &values);
    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}
*/

QString CoreDB::getAlbumRelativePath(int albumID)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT relativePath from Albums WHERE id=?"),
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

int CoreDB::getAlbumRootId(int albumID)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT albumRoot FROM Albums WHERE id=?; "),
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

QDate CoreDB::getAlbumLowestDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT MIN(creationDate) FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.album=? GROUP BY Images.album;"),
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

QDate CoreDB::getAlbumHighestDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT MAX(creationDate) FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.album=? GROUP BY Images.album;"),
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

QDate CoreDB::getAlbumAverageDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT creationDate FROM ImageInformation "
                   " INNER JOIN Images ON Images.id=ImageInformation.imageid "
                   " WHERE Images.album=?;"),
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

void CoreDB::deleteItem(int albumID, const QString& file)
{
    qlonglong imageId = getImageId(albumID, file);

    d->db->execSql(QString::fromUtf8("DELETE FROM Images WHERE id=?;"),
                   imageId);

    d->db->recordChangeset(CollectionImageChangeset(imageId, albumID, CollectionImageChangeset::Deleted));
}

void CoreDB::deleteItem(qlonglong imageId)
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Images WHERE id=? AND album IS NULL;"),
                   imageId);

}

void CoreDB::removeItemsFromAlbum(int albumID, const QList<qlonglong>& ids_forInformation)
{
    d->db->execSql(QString::fromUtf8("UPDATE Images SET status=?, album=NULL WHERE album=?;"),
                   (int)DatabaseItem::Trashed, albumID);

    d->db->recordChangeset(CollectionImageChangeset(ids_forInformation, albumID, CollectionImageChangeset::RemovedAll));
}

void CoreDB::removeItems(QList<qlonglong> itemIDs, const QList<int>& albumIDs)
{
    DbEngineSqlQuery query = d->db->prepareQuery(QString::fromUtf8("UPDATE Images SET status=?, album=NULL WHERE id=?;"));

    QVariantList imageIds;
    QVariantList status;

    foreach(const qlonglong& id, itemIDs)
    {
        status << (int)DatabaseItem::Trashed;
        imageIds << id;
    }

    query.addBindValue(status);
    query.addBindValue(imageIds);
    d->db->execBatch(query);

    d->db->recordChangeset(CollectionImageChangeset(itemIDs, albumIDs, CollectionImageChangeset::Removed));
}

void CoreDB::removeItemsPermanently(QList<qlonglong> itemIDs, const QList<int>& albumIDs)
{
    DbEngineSqlQuery query = d->db->prepareQuery(QString::fromUtf8("UPDATE Images SET status=?, album=NULL WHERE id=?;"));

    QVariantList imageIds;
    QVariantList status;

    foreach(const qlonglong& id, itemIDs)
    {
        status << (int)DatabaseItem::Obsolete;
        imageIds << id;
    }

    query.addBindValue(status);
    query.addBindValue(imageIds);
    d->db->execBatch(query);

    d->db->recordChangeset(CollectionImageChangeset(itemIDs, albumIDs, CollectionImageChangeset::Removed));
}

void CoreDB::deleteRemovedItems()
{
    d->db->execSql(QString::fromUtf8("DELETE FROM Images WHERE status=?;"),
                   (int)DatabaseItem::Obsolete);

    d->db->recordChangeset(CollectionImageChangeset(QList<qlonglong>(), QList<int>(), CollectionImageChangeset::RemovedDeleted));
}

/*
// This method is probably nonsense because a remove image no longer has an associated album
void CoreDB::deleteRemovedItems(QList<int> albumIds)
{
    DbEngineSqlQuery query = d->db->prepareQuery( QString::fromUtf8("DELETE FROM Images WHERE status=? AND album=?;") );

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

void CoreDB::renameAlbum(int albumID, int newAlbumRoot, const QString& newRelativePath)
{
    int albumRoot        = getAlbumRootId(albumID);
    QString relativePath = getAlbumRelativePath(albumID);

    if (relativePath == newRelativePath && albumRoot == newAlbumRoot)
    {
        return;
    }

    // first delete any stale albums left behind at the destination of renaming
    QMap<QString, QVariant> parameters;
    parameters.insert(QString::fromUtf8(":albumRoot"),    newAlbumRoot);
    parameters.insert(QString::fromUtf8(":relativePath"), newRelativePath);

    if (BdEngineBackend::NoErrors != d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("deleteAlbumRootPath")), parameters))
    {
        return;
    }

    // now update the album
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET albumRoot=?, relativePath=? WHERE id=? AND albumRoot=?;"),
                   newAlbumRoot, newRelativePath, albumID, albumRoot);
    d->db->recordChangeset(AlbumChangeset(albumID, AlbumChangeset::Renamed));

/*
    if (renameSubalbums)
    {
        // now find the list of all subalbums which need to be updated
        QList<QVariant> values;
        d->db->execSql( QString::fromUtf8("SELECT id, relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE ?;"),
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
            d->db->execSql(QString::fromUtf8("UPDATE Albums SET albumRoot=?, relativePath=? WHERE albumRoot=? AND relativePath=?"),
                           newAlbumRoot, newChildURL, albumRoot, (*it) );
            d->db->recordChangeset(AlbumChangeset(childAlbumId, AlbumChangeset::Renamed));
        }
    }
*/
}

void CoreDB::setTagName(int tagID, const QString& name)
{
    d->db->execSql(QString::fromUtf8("UPDATE Tags SET name=? WHERE id=?;"),
                   name, tagID);
    d->db->recordChangeset(TagChangeset(tagID, TagChangeset::Renamed));
}

void CoreDB::moveItem(int srcAlbumID, const QString& srcName,
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

    d->db->execSql(QString::fromUtf8("UPDATE Images SET album=?, name=? "
                                     "WHERE id=?;"),
                   dstAlbumID, dstName, imageId);
    d->db->recordChangeset(CollectionImageChangeset(imageId, srcAlbumID, CollectionImageChangeset::Moved));
    d->db->recordChangeset(CollectionImageChangeset(imageId, srcAlbumID, CollectionImageChangeset::Removed));
    d->db->recordChangeset(CollectionImageChangeset(imageId, dstAlbumID, CollectionImageChangeset::Added));
}

int CoreDB::copyItem(int srcAlbumID, const QString& srcName,
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
    d->db->execSql(QString::fromUtf8("INSERT INTO Images "
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

void CoreDB::copyImageAttributes(qlonglong srcId, qlonglong dstId)
{
    // Go through all image-specific tables and copy the entries

    DatabaseFields::Set fields;

    d->db->execSql(QString::fromUtf8("INSERT INTO ImageInformation "
                           " (imageid, rating, creationDate, digitizationDate, orientation, "
                           "  width, height, format, colorDepth, colorModel) "
                           "SELECT ?, rating, creationDate, digitizationDate, orientation, "
                           "  width, height, format, colorDepth, colorModel "
                           "FROM ImageInformation WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageInformationAll;

    d->db->execSql(QString::fromUtf8("INSERT INTO ImageMetadata "
                           " (imageid, make, model, lens, aperture, focalLength, focalLength35, "
                           "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                           "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) "
                           "SELECT ?, make, model, lens, aperture, focalLength, focalLength35, "
                           "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                           "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory "
                           "FROM ImageMetadata WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageMetadataAll;

    d->db->execSql(QString::fromUtf8("INSERT INTO VideoMetadata "
                           " (imageid, aspectRatio, audioBitRate, audioChannelType, audioCompressor, duration, frameRate, "
                           "  videoCodec) "
                           "SELECT ?, aspectRatio, audioBitRate, audioChannelType, audioCompressor, duration, frameRate, "
                           "  videoCodec "
                           "FROM VideoMetadata WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::VideoMetadataAll;

    d->db->execSql(QString::fromUtf8("INSERT INTO ImagePositions "
                           " (imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                           "  altitude, orientation, tilt, roll, accuracy, description) "
                           "SELECT ?, latitude, latitudeNumber, longitude, longitudeNumber, "
                           "  altitude, orientation, tilt, roll, accuracy, description "
                           "FROM ImagePositions WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImagePositionsAll;

    d->db->execSql(QString::fromUtf8("INSERT INTO ImageComments "
                           " (imageid, type, language, author, date, comment) "
                           "SELECT ?, type, language, author, date, comment "
                           "FROM ImageComments WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageCommentsAll;

    d->db->execSql(QString::fromUtf8("INSERT INTO ImageCopyright "
                           " (imageid, property, value, extraValue) "
                           "SELECT ?, property, value, extraValue "
                           "FROM ImageCopyright WHERE imageid=?;"),
                   dstId, srcId);

    d->db->execSql(QString::fromUtf8("INSERT INTO ImageHistory "
                           " (imageid, uuid, history) "
                           "SELECT ?, uuid, history "
                           "FROM ImageHistory WHERE imageid=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageHistoryInfoAll;

    d->db->execSql(QString::fromUtf8("INSERT INTO ImageRelations "
                           " (subject, object, type) "
                           "SELECT ?, object, type "
                           "FROM ImageRelations WHERE subject=?;"),
                   dstId, srcId);
    d->db->execSql(QString::fromUtf8("INSERT INTO ImageRelations "
                           " (subject, object, type) "
                           "SELECT subject, ?, type "
                           "FROM ImageRelations WHERE object=?;"),
                   dstId, srcId);
    fields |= DatabaseFields::ImageRelations;

    d->db->recordChangeset(ImageChangeset(dstId, fields));

    copyImageTags(srcId, dstId);
    copyImageProperties(srcId, dstId);
}

void CoreDB::copyImageProperties(qlonglong srcId, qlonglong dstId)
{
    d->db->execSql(QString::fromUtf8("INSERT INTO ImageProperties "
                           " (imageid, property, value) "
                           "SELECT ?, property, value "
                           "FROM ImageProperties WHERE imageid=?;"),
                   dstId, srcId);
}

void CoreDB::copyImageTags(qlonglong srcId, qlonglong dstId)
{
    d->db->execSql(QString::fromUtf8("INSERT INTO ImageTags "
                           " (imageid, tagid) "
                           "SELECT ?, tagid "
                           "FROM ImageTags WHERE imageid=?;"),
                   dstId, srcId);

    d->db->execSql(QString::fromUtf8("INSERT INTO ImageTagProperties "
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

bool CoreDB::copyAlbumProperties(int srcAlbumID, int dstAlbumID)
{
    if (srcAlbumID == dstAlbumID)
    {
        return true;
    }

    QList<QVariant> values;
    d->db->execSql(QString::fromUtf8("SELECT date, caption, collection, icon "
                           "FROM Albums WHERE id=?;"),
                   srcAlbumID,
                   &values);

    if (values.isEmpty())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << " src album ID " << srcAlbumID << " does not exist";
        return false;
    }

    QList<QVariant> boundValues;
    boundValues << values.at(0) << values.at(1) << values.at(2) << values.at(3);
    boundValues << dstAlbumID;

    //shouldn't we have a ; at the end of the query?
    d->db->execSql(QString::fromUtf8("UPDATE Albums SET date=?, caption=?, "
                           "collection=?, icon=? WHERE id=?"),
                   boundValues);
    return true;
}

QList<QVariant> CoreDB::getImageIdsFromArea(qreal lat1, qreal lat2, qreal lng1, qreal lng2, int /*sortMode*/,
                                             const QString& /*sortBy*/)
{
    QList<QVariant> values;
    QList<QVariant> boundValues;
    boundValues << lat1 << lat2 << lng1 << lng2;

    //CoreDbAccess access;

    d->db->execSql(QString::fromUtf8("Select ImageInformation.imageid, ImageInformation.rating, ImagePositions.latitudeNumber, ImagePositions.longitudeNumber"
                           " FROM ImageInformation INNER JOIN ImagePositions"
                           " ON ImageInformation.imageid = ImagePositions.imageid"
                           " WHERE (ImagePositions.latitudeNumber>? AND ImagePositions.latitudeNumber<?)"
                           " AND (ImagePositions.longitudeNumber>? AND ImagePositions.longitudeNumber<?);"),
                   boundValues, &values);

    return values;
}

bool CoreDB::integrityCheck()
{
    QList<QVariant> values;
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("checkCoreDbIntegrity")), &values);
    switch (d->db->databaseType())
    {
        case BdEngineBackend::DbType::SQLite:
            // For SQLite the integrity check returns a single row with one string column "ok" on success and multiple rows on error.
            return values.size() == 1 && values.first().toString().toLower().compare(QLatin1String("ok")) == 0;
        case BdEngineBackend::DbType::MySQL:
            // For MySQL, for every checked table, the table name, operation (check), message type (status) and the message text (ok on success)
            // are returned. So we check if there are four elements and if yes, whether the fourth element is "ok".
            //qCDebug(DIGIKAM_DATABASE_LOG) << "MySQL check returned " << values.size() << " rows";
            if ( (values.size() % 4) != 0)
            {
                return false;
            }

            for (QList<QVariant>::iterator it = values.begin(); it != values.end(); )
            {
                QString tableName   = (*it).toString();
                ++it;
                QString operation   = (*it).toString();
                ++it;
                QString messageType = (*it).toString();
                ++it;
                QString messageText = (*it).toString();
                ++it;

                if (messageText.toLower().compare(QLatin1String("ok")) != 0)
                {
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Failed integrity check for table " << tableName << ". Reason:" << messageText;
                    return false;
                }
                else
                {
                    //qCDebug(DIGIKAM_DATABASE_LOG) << "Passed integrity check for table " << tableName;
                }
            }
            // No error conditions. Db passed the integrity check.
            return true;
        default:
            return false;
    }
}

void CoreDB::vacuum()
{
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("vacuumCoreDB")));
}

void CoreDB::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->recentlyAssignedTags = group.readEntry(d->configRecentlyUsedTags, QList<int>());
}

void CoreDB::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configRecentlyUsedTags, d->recentlyAssignedTags);
}

}  // namespace Digikam
