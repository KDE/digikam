/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnails database interface.
 *
 * Copyright (C)      2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbsdb.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QDir>

// Local includes

#include "digikam_debug.h"
#include "collectionmanager.h"
#include "collectionlocation.h"

namespace Digikam
{

class ThumbsDb::Private
{

public:

    Private() :
        db(0)
    {
    }

    ThumbsDbBackend* db;
};

ThumbsDb::ThumbsDb(ThumbsDbBackend* const backend)
    : d(new Private)
{
    d->db = backend;
}

ThumbsDb::~ThumbsDb()
{
    delete d;
}

bool ThumbsDb::setSetting(const QString& keyword, const QString& value )
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    parameters.insert(QLatin1String(":value"), value);
    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("ReplaceThumbnailSetting")), parameters);

    return (queryStateResult == BdEngineBackend::NoErrors);
}

QString ThumbsDb::getSetting(const QString& keyword)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    QList<QVariant> values;
    // TODO Should really check return status here
    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("SelectThumbnailSetting")), parameters, &values);
    qCDebug(DIGIKAM_THUMBSDB_LOG) << "ThumbDB SelectThumbnailSetting val ret = " << (BdEngineBackend::QueryStateEnum)queryStateResult;

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

QString ThumbsDb::getLegacySetting(const QString& keyword)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    QList<QVariant> values;
    // TODO Should really check return status here
    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("SelectThumbnailLegacySetting")), parameters, &values);
    qCDebug(DIGIKAM_THUMBSDB_LOG) << "ThumbDB SelectThumbnailLegacySetting val ret = " << (BdEngineBackend::QueryStateEnum)queryStateResult;

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

static void fillThumbnailInfo(const QList<QVariant> &values, ThumbsDbInfo& info)
{
    if (values.isEmpty())
    {
        return;
    }

    info.id               = values.at(0).toInt();
    info.type             = (DatabaseThumbnail::Type)values.at(1).toInt();
    info.modificationDate = values.at(2).isNull() ? QDateTime() : QDateTime::fromString(values.at(2).toString(), Qt::ISODate);
    info.orientationHint  = values.at(3).toInt();
    info.data             = values.at(4).toByteArray();
}

ThumbsDbInfo ThumbsDb::findByHash(const QString& uniqueHash, qlonglong fileSize)
{
    QList<QVariant> values;
    d->db->execSql(QLatin1String("SELECT id, type, modificationDate, orientationHint, data "
                                 "FROM UniqueHashes "
                                 "   INNER JOIN Thumbnails ON thumbId = id "
                                 "WHERE uniqueHash=? AND fileSize=?;"),
                   uniqueHash, fileSize,
                   &values );

    ThumbsDbInfo info;
    fillThumbnailInfo(values, info);
    return info;
}

ThumbsDbInfo ThumbsDb::findByFilePath(const QString& path)
{
    QList<QVariant> values;
    d->db->execSql(QLatin1String("SELECT id, type, modificationDate, orientationHint, data "
                                 "FROM FilePaths "
                                 "   INNER JOIN Thumbnails ON thumbId = id "
                                 "WHERE path=?;"),
                   path,
                   &values );

    ThumbsDbInfo info;
    fillThumbnailInfo(values, info);

    return info;
}

ThumbsDbInfo ThumbsDb::findByFilePath(const QString& path, const QString& uniqueHash)
{
    ThumbsDbInfo info = findByFilePath(path);

    if (uniqueHash.isNull())
    {
        return info;
    }

    if (info.data.isNull())
    {
        return info;
    }

    // double check that thumbnail is not referenced by a different hash
    QList<QVariant> values;
    d->db->execSql(QLatin1String("SELECT uniqueHash FROM UniqueHashes WHERE thumbId=?;"),
                   info.id, &values);

    if (values.isEmpty())
    {
        return info;
    }
    else
    {
        foreach(const QVariant& hash, values)
        {
            if (hash == uniqueHash)
            {
                return info;
            }
        }

        return ThumbsDbInfo();
    }
}

ThumbsDbInfo ThumbsDb::findByCustomIdentifier(const QString& id)
{
    QList<QVariant> values;
    d->db->execSql(QLatin1String("SELECT id, type, modificationDate, orientationHint, data "
                                 "FROM CustomIdentifiers "
                                 "   INNER JOIN Thumbnails ON thumbId = id "
                                 "WHERE identifier=?;"),
                   id,
                   &values);

    ThumbsDbInfo info;
    fillThumbnailInfo(values, info);
    return info;
}

QList<int> ThumbsDb::findAll()
{
    QList<QVariant> values;
    d->db->execSql(QLatin1String("SELECT id FROM Thumbnails;"),&values);

    QList<int> thumbIds;
    foreach (QVariant object, values)
    {
        thumbIds << object.toInt();
    }
    return thumbIds;
}

QHash<QString, int> ThumbsDb::getFilePathsWithThumbnail()
{
    DbEngineSqlQuery query = d->db->prepareQuery(QString::fromLatin1("SELECT path, id "
                                                             "FROM FilePaths "
                                                             "   INNER JOIN Thumbnails ON FilePaths.thumbId=Thumbnails.id "
                                                             "WHERE type BETWEEN %1 AND %2;")
                                         .arg(DatabaseThumbnail::PGF)
                                         .arg(DatabaseThumbnail::PNG));

    if (!d->db->exec(query))
    {
        return QHash<QString, int>();
    }

    QHash <QString, int> filePaths;

    while (query.next())
    {
        filePaths[query.value(0).toString()] = query.value(1).toInt();
    }

    return filePaths;
}

BdEngineBackend::QueryState ThumbsDb::insertUniqueHash(const QString& uniqueHash, qlonglong fileSize, int thumbId)
{
    return d->db->execSql(QLatin1String("REPLACE INTO UniqueHashes (uniqueHash, fileSize, thumbId) VALUES (?,?,?);"),
                          uniqueHash, fileSize, thumbId);
}

BdEngineBackend::QueryState ThumbsDb::insertFilePath(const QString& path, int thumbId)
{
    return d->db->execSql(QLatin1String("REPLACE INTO FilePaths (path, thumbId) VALUES (?,?);"),
                          path, thumbId);
}

BdEngineBackend::QueryState ThumbsDb::insertCustomIdentifier(const QString& path, int thumbId)
{
    return d->db->execSql(QLatin1String("REPLACE INTO CustomIdentifiers (identifier, thumbId) VALUES (?,?);"),
                          path, thumbId);
}

BdEngineBackend::QueryState ThumbsDb::remove(int thumbId)
{
    return d->db->execSql(QLatin1String("DELETE FROM Thumbnails WHERE id=?;"), thumbId);
}

BdEngineBackend::QueryState ThumbsDb::removeByUniqueHash(const QString& uniqueHash, qlonglong fileSize)
{
    // UniqueHashes + FilePaths entries are removed by trigger
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":uniqueHash"), uniqueHash);
    parameters.insert(QLatin1String(":filesize"),   fileSize);

    return d->db->execDBAction(d->db->getDBAction(QLatin1String("Delete_Thumbnail_ByUniqueHashId")), parameters);
}

BdEngineBackend::QueryState ThumbsDb::removeByFilePath(const QString& path)
{
    // UniqueHashes + FilePaths entries are removed by trigger
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":path"), path);

    return d->db->execDBAction(d->db->getDBAction(QLatin1String("Delete_Thumbnail_ByPath")), parameters);
}

BdEngineBackend::QueryState ThumbsDb::removeByCustomIdentifier(const QString& id)
{
    // UniqueHashes + FilePaths entries are removed by trigger
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":identifier"), id);

    return d->db->execDBAction(d->db->getDBAction(QLatin1String("Delete_Thumbnail_ByCustomIdentifier")), parameters);
}

BdEngineBackend::QueryState ThumbsDb::insertThumbnail(const ThumbsDbInfo& info, QVariant* const lastInsertId)
{
    QVariant id;
    BdEngineBackend::QueryState lastQueryState;
    lastQueryState = d->db->execSql(QLatin1String("INSERT INTO Thumbnails (type, modificationDate, orientationHint, data) VALUES (?, ?, ?, ?);"),
                                    info.type, info.modificationDate, info.orientationHint, info.data,
                                    0, &id);

    if (BdEngineBackend::NoErrors == lastQueryState)
    {
        *lastInsertId=id.toInt();
    }
    else
    {
        *lastInsertId=-1;
    }

    return lastQueryState;
}

BdEngineBackend::QueryState ThumbsDb::replaceThumbnail(const ThumbsDbInfo& info)
{
    return d->db->execSql(QLatin1String("REPLACE INTO Thumbnails (id, type, modificationDate, orientationHint, data) VALUES(?, ?, ?, ?, ?);"),
                          QList<QVariant>() << info.id << info.type << info.modificationDate << info.orientationHint << info.data);
}

BdEngineBackend::QueryState ThumbsDb::updateModificationDate(int thumbId, const QDateTime& modificationDate)
{
    return d->db->execSql(QLatin1String("UPDATE Thumbnails SET modificationDate=? WHERE id=?;"), modificationDate, thumbId);
}

void ThumbsDb::replaceUniqueHash(const QString& oldUniqueHash, int oldFileSize,
                                 const QString& newUniqueHash, int newFileSize)
{
    d->db->execSql(QLatin1String("UPDATE UniqueHashes SET uniqueHash=?, fileSize=? WHERE uniqueHash=? AND fileSize=?;"),
                   newUniqueHash, newFileSize, oldUniqueHash, oldFileSize);
}

bool ThumbsDb::integrityCheck()
{
    QList<QVariant> values;
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("checkThumbnailsDbIntegrity")), &values);
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

void ThumbsDb::vacuum()
{
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("vacuumThumbnailsDB")));
}

}  // namespace Digikam
