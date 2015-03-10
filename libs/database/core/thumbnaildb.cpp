/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : database thumbnail interface.
 *
 * Copyright (C)      2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbnaildb.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QDir>

// KDE includes

#include <klocale.h>

// Local includes

#include "databasecorebackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"

namespace Digikam
{

class ThumbnailDB::Private
{

public:

    Private() :
        db(0)
    {
    }

    DatabaseCoreBackend* db;
};

ThumbnailDB::ThumbnailDB(DatabaseCoreBackend* const backend)
    : d(new Private)
{
    d->db = backend;
}

ThumbnailDB::~ThumbnailDB()
{
    delete d;
}

bool ThumbnailDB::setSetting(const QString& keyword, const QString& value )
{
    return  d->db->execSql( "REPLACE INTO Settings VALUES (?,?);",
                            keyword, value );
}

QString ThumbnailDB::getSetting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql( "SELECT value FROM Settings WHERE keyword=?;",
                    keyword, &values );

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

static void fillThumbnailInfo(const QList<QVariant> &values, DatabaseThumbnailInfo& info)
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

DatabaseThumbnailInfo ThumbnailDB::findByHash(const QString& uniqueHash, qlonglong fileSize)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id, type, modificationDate, orientationHint, data "
                            "FROM UniqueHashes "
                            "   INNER JOIN Thumbnails ON thumbId = id "
                            "WHERE uniqueHash=? AND fileSize=?;"),
                    uniqueHash, fileSize,
                    &values );

    DatabaseThumbnailInfo info;
    fillThumbnailInfo(values, info);
    return info;
}

DatabaseThumbnailInfo ThumbnailDB::findByFilePath(const QString& path)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id, type, modificationDate, orientationHint, data "
                            "FROM FilePaths "
                            "   INNER JOIN Thumbnails ON thumbId = id "
                            "WHERE path=?;"),
                    path,
                    &values );

    DatabaseThumbnailInfo info;
    fillThumbnailInfo(values, info);
    return info;
}

DatabaseThumbnailInfo ThumbnailDB::findByFilePath(const QString& path, const QString& uniqueHash)
{
    DatabaseThumbnailInfo info = findByFilePath(path);

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
    d->db->execSql("SELECT uniqueHash FROM UniqueHashes WHERE thumbId=?;",
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
        return DatabaseThumbnailInfo();
    }
}

DatabaseThumbnailInfo ThumbnailDB::findByCustomIdentifier(const QString& id)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id, type, modificationDate, orientationHint, data "
                            "FROM CustomIdentifiers "
                            "   INNER JOIN Thumbnails ON thumbId = id "
                            "WHERE identifier=?;"),
                    id,
                    &values );

    DatabaseThumbnailInfo info;
    fillThumbnailInfo(values, info);
    return info;
}

QHash<QString, int> ThumbnailDB::getFilePathsWithThumbnail()
{
    SqlQuery query = d->db->prepareQuery(QString("SELECT path, id "
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

DatabaseCoreBackend::QueryState ThumbnailDB::insertUniqueHash(const QString& uniqueHash, qlonglong fileSize, int thumbId)
{
    return d->db->execSql("REPLACE INTO UniqueHashes (uniqueHash, fileSize, thumbId) VALUES (?,?,?)",
                          uniqueHash, fileSize, thumbId);
}

DatabaseCoreBackend::QueryState ThumbnailDB::insertFilePath(const QString& path, int thumbId)
{
    return d->db->execSql("REPLACE INTO FilePaths (path, thumbId) VALUES (?,?)",
                          path, thumbId);
}

DatabaseCoreBackend::QueryState ThumbnailDB::insertCustomIdentifier(const QString& path, int thumbId)
{
    return d->db->execSql("REPLACE INTO CustomIdentifiers (identifier, thumbId) VALUES (?,?)",
                          path, thumbId);
}

DatabaseCoreBackend::QueryState ThumbnailDB::removeByUniqueHash(const QString& uniqueHash, qlonglong fileSize)
{
    // UniqueHashes + FilePaths entries are removed by trigger
    QMap<QString, QVariant> parameters;
    parameters.insert(":uniqueHash", uniqueHash);
    parameters.insert(":filesize", fileSize);
    return d->db->execDBAction(d->db->getDBAction(QString("Delete_Thumbnail_ByUniqueHashId")), parameters);
}

DatabaseCoreBackend::QueryState ThumbnailDB::removeByFilePath(const QString& path)
{
    // UniqueHashes + FilePaths entries are removed by trigger
    QMap<QString, QVariant> parameters;
    parameters.insert(":path", path);
    return d->db->execDBAction(d->db->getDBAction(QString("Delete_Thumbnail_ByPath")), parameters);
}

DatabaseCoreBackend::QueryState ThumbnailDB::removeByCustomIdentifier(const QString& id)
{
    // UniqueHashes + FilePaths entries are removed by trigger
    QMap<QString, QVariant> parameters;
    parameters.insert(":identifier", id);
    return d->db->execDBAction(d->db->getDBAction(QString("Delete_Thumbnail_ByCustomIdentifier")), parameters);
}

DatabaseCoreBackend::QueryState ThumbnailDB::insertThumbnail(const DatabaseThumbnailInfo& info, QVariant* const lastInsertId)
{
    QVariant id;
    DatabaseCoreBackend::QueryState lastQueryState;
    lastQueryState = d->db->execSql("INSERT INTO Thumbnails (type, modificationDate, orientationHint, data) VALUES (?, ?, ?, ?);",
                                    info.type, info.modificationDate, info.orientationHint, info.data,
                                    0, &id);

    if (DatabaseCoreBackend::NoErrors==lastQueryState)
    {
        *lastInsertId=id.toInt();
    }
    else
    {
        *lastInsertId=-1;
    }

    return lastQueryState;
}

DatabaseCoreBackend::QueryState ThumbnailDB::replaceThumbnail(const DatabaseThumbnailInfo& info)
{
    return d->db->execSql("REPLACE INTO Thumbnails (id, type, modificationDate, orientationHint, data) VALUES(?, ?, ?, ?, ?);",
                          QList<QVariant>() << info.id << info.type << info.modificationDate << info.orientationHint << info.data);
}

DatabaseCoreBackend::QueryState ThumbnailDB::updateModificationDate(int thumbId, const QDateTime& modificationDate)
{
    return d->db->execSql("UPDATE Thumbnails SET modificationDate=? WHERE id=?;", modificationDate, thumbId);
}

void ThumbnailDB::replaceUniqueHash(const QString& oldUniqueHash, int oldFileSize,
                                    const QString& newUniqueHash, int newFileSize)
{
    d->db->execSql("UPDATE UniqueHashes SET uniqueHash=?, fileSize=? WHERE uniqueHash=? AND fileSize=?",
                   newUniqueHash, newFileSize, oldUniqueHash, oldFileSize);
}

}  // namespace Digikam
