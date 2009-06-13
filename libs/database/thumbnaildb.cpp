/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : database thumbnail interface.
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "databasecorebackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"

namespace Digikam
{

class ThumbnailDBPriv
{

public:

    ThumbnailDBPriv()
    {
        db = 0;
    }

    DatabaseCoreBackend *db;
};

ThumbnailDB::ThumbnailDB(DatabaseCoreBackend *backend)
           : d(new ThumbnailDBPriv)
{
    d->db = backend;
}

ThumbnailDB::~ThumbnailDB()
{
    delete d;
}

void ThumbnailDB::setSetting(const QString& keyword,
                         const QString& value )
{
    d->db->execSql( "REPLACE INTO Settings VALUES (?,?);",
                    keyword, value );
}

QString ThumbnailDB::getSetting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql( "SELECT value FROM Settings WHERE keyword=?;",
                    keyword, &values );

    if (values.isEmpty())
        return QString();
    else
        return values.first().toString();
}

static void fillThumbnailInfo(const QList<QVariant> &values, DatabaseThumbnailInfo &info)
{
    if (values.isEmpty())
        return;
    info.id               = values[0].toInt();
    info.type             = (DatabaseThumbnail::Type)values[1].toInt();
    info.modificationDate = values[2].isNull() ? QDateTime() : QDateTime::fromString(values[2].toString(), Qt::ISODate);
    info.orientationHint  = values[3].toInt();
    info.data             = values[4].toByteArray();
}

DatabaseThumbnailInfo ThumbnailDB::findByHash(const QString &uniqueHash, int fileSize)
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

DatabaseThumbnailInfo ThumbnailDB::findByFilePath(const QString &path)
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

QStringList ThumbnailDB::getValidFilePaths()
{
    QList<QVariant> values;
    QSqlQuery query;
    query = d->db->prepareQuery(QString("SELECT path "
                                        "FROM FilePaths "
                                        "   INNER JOIN Thumbnails ON FilePaths.thumbId=Thumbnails.id "
                                        "WHERE Thumbnails.type NOT IN(%1,%2);")
                                .arg(DatabaseThumbnail::UndefinedType)
                                .arg(DatabaseThumbnail::NoThumbnail));

    if (!d->db->exec(query))
        return QStringList();

    QStringList paths;

    while (query.next())
    {
        paths << query.value(0).toString();
    }
    return paths;
}

void ThumbnailDB::insertUniqueHash(const QString &uniqueHash, int fileSize, int thumbId)
{
    d->db->execSql("REPLACE INTO UniqueHashes (uniqueHash, fileSize, thumbId) VALUES (?,?,?)",
                   uniqueHash, fileSize, thumbId);
}

void ThumbnailDB::insertFilePath(const QString &path, int thumbId)
{
    d->db->execSql("REPLACE INTO FilePaths (path, thumbId) VALUES (?,?)",
                   path, thumbId);
}

void ThumbnailDB::removeByUniqueHash(const QString &uniqueHash, int fileSize)
{
    // UniqueHashes + FilePaths entries are removed by trigger
    d->db->execSql("DELETE FROM Thumbnails WHERE id IN "
                   " (SELECT thumbId FROM UniqueHashes WHERE uniqueHash=? AND fileSize=?);",
                   uniqueHash, fileSize);
}

void ThumbnailDB::removeByFilePath(const QString &path)
{
    // UniqueHashes + FilePaths entries are removed by trigger
    d->db->execSql("DELETE FROM Thumbnails WHERE id IN "
                   " (SELECT thumbId FROM FilePaths WHERE path=?);",
                   path);
}

int ThumbnailDB::insertThumbnail(const DatabaseThumbnailInfo &info)
{
    QVariant id;
    if (!d->db->execSql("INSERT INTO Thumbnails (type, modificationDate, orientationHint, data) VALUES (?, ?, ?, ?);",
                        info.type, info.modificationDate, info.orientationHint, info.data,
                        0, &id) )
    {
        return -1;
    }

    return id.toInt();
}

void ThumbnailDB::replaceThumbnail(const DatabaseThumbnailInfo &info)
{
    d->db->execSql("REPLACE INTO THUMBNAILS (id, type, modificationDate, orientationHint, data) VALUES(?, ?, ?, ?, ?);",
                    QList<QVariant>() << info.id << info.type << info.modificationDate << info.orientationHint << info.data);
}

}  // namespace Digikam
