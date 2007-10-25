/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cerrno>

// Qt includes.

#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QRegExp>
#include <QDir>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <kdebug.h>

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionlocation.h"
#include "dmetadata.h"
#include "imagelister.h"

namespace Digikam
{

QDataStream &operator<<(QDataStream &os, const ImageListerRecord &record)
{
    os << record.imageID;
    os << record.albumID;
    os << record.albumRootID;
    os << record.name;

    os << record.rating;
    os << record.creationDate;
    os << record.modificationDate;
    os << record.fileSize;
    os << record.imageSize;

    return os;
}

QDataStream &operator>>(QDataStream &ds, ImageListerRecord &record)
{
    ds >> record.imageID;
    ds >> record.albumID;
    ds >> record.albumRootID;
    ds >> record.name;

    ds >> record.rating;
    ds >> record.creationDate;
    ds >> record.modificationDate;
    ds >> record.fileSize;
    ds >> record.imageSize;

    return ds;
}

KIO::TransferJob *ImageLister::startListJob(const DatabaseUrl &url, int extraValue)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << url;
    if (extraValue != -1)
        ds << extraValue;

    return new KIO::SpecialJob(url, ba);
}

KIO::TransferJob *ImageLister::startScanJob(const DatabaseUrl &url, const QString &filter, int extraValue)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << url;
    ds << filter;
    if (extraValue != -1)
        ds << extraValue;

    return new KIO::SpecialJob(url, ba);
}

QSize ImageLister::retrieveDimension(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString rawFilesExt(raw_file_extentions);
    QString ext = fileInfo.suffix().toUpper();

    if (!ext.isEmpty() && rawFilesExt.toUpper().contains(ext))
    {
        Digikam::DMetadata metaData(filePath);
        return metaData.getImageDimensions();
    }
    else
    {
        KFileMetaInfo metaInfo(filePath, QString(), KFileMetaInfo::TechnicalInfo);
        if (metaInfo.isValid())
        {
            //TODO: KDE4PORT: Find out the correct key values. Strigi analyzers are used.
            // If necessary, use exiv2 and/or strigi directly.
            KFileMetaInfoItem itemWidth = metaInfo.item("image.width");
            KFileMetaInfoItem itemHeight = metaInfo.item("image.height");
            if (itemWidth.isValid() && itemHeight.isValid())
            {
                return QSize(itemWidth.value().toInt(), itemHeight.value().toInt());
            }
        }
    }
    return QSize();
}


void ImageLister::list(ImageListerReceiver *receiver, const DatabaseUrl &url)
{
    if (url.isAlbumUrl())
    {
        QString albumRoot = url.albumRootPath();
        QString album     = url.album();
        listAlbum(receiver, albumRoot, album);
    }
    else if (url.isTagUrl())
    {
        listTag(receiver, url.tagId());
    }
    else if (url.isDateUrl())
    {
        listMonth(receiver, url.date());
    }
}

void ImageLister::listAlbum(ImageListerReceiver *receiver,
                            const QString &albumRoot, const QString &album)
{
    CollectionLocation *location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);
    if (!location)
        return;

    int albumid = DatabaseAccess().db()->getAlbumForPath(albumRoot, album, false);
    if (albumid == -1)
        return;

    listAlbum(receiver, albumRoot, location->id(), album, albumid);
}

void ImageLister::listAlbum(ImageListerReceiver *receiver,
                            const QString &albumRoot, int albumRootID, const QString &album, int albumId)
{
    QString base      = albumRoot + album;

    QList<QVariant> values;

    {
        DatabaseAccess access;
        access.backend()->execSql(QString("SELECT DISTINCT Images.id, Images.name, "
                                          "       ImageInformation.rating, ImageInformation.creationDate, "
                                          "       Images.modificationDate, Images.fileSize, "
                                          "       ImageInformation.width, ImageInformation.height "
                                          " FROM Images "
                                          "       LEFT OUTER JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                                          " WHERE Images.album = ?;"),
                                  albumId, &values);
    }

    int width, height;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID           = (*it).toLongLong();
        ++it;
        record.name              = (*it).toString();
        ++it;
        record.rating            = (*it).toInt();
        ++it;
        record.creationDate      = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.modificationDate  = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.fileSize          = (*it).toInt();
        ++it;
        width                    = (*it).toInt();
        ++it;
        height                   = (*it).toInt();
        ++it;

        record.imageSize         = QSize(width, height);

        record.albumID     = albumId;
        record.albumRootID = albumRootID;

        receiver->receive(record);
    }
}

void ImageLister::listTag(ImageListerReceiver *receiver, int tagId)
{
    QList<QVariant> values;

    {
        DatabaseAccess access;
        access.backend()->execSql( QString( "SELECT DISTINCT Images.id, Images.name, Images.album, "
                                            "       Albums.albumRoot, "
                                            "       ImageInformation.rating, ImageInformation.creationDate, "
                                            "       Images.modificationDate, Images.fileSize, "
                                            "       ImageInformation.width, ImageInformation.height "
                                            " FROM Images "
                                            "       LEFT OUTER JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                                            "       LEFT OUTER JOIN Albums ON Albums.id=Images.album "
                                            " WHERE Images.id IN "
                                            "       (SELECT imageid FROM ImageTags "
                                            "        WHERE tagid=? "
                                            "           OR tagid IN (SELECT id FROM TagsTree WHERE pid=?)); "),
                                    tagId, tagId, &values );
    }

    int width, height;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;

        record.imageID           = (*it).toLongLong();
        ++it;
        record.name              = (*it).toString();
        ++it;
        record.albumID           = (*it).toInt();
        ++it;
        record.albumRootID       = (*it).toInt();
        ++it;
        record.rating            = (*it).toInt();
        ++it;
        record.creationDate      = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.modificationDate  = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.fileSize          = (*it).toInt();
        ++it;
        width                    = (*it).toInt();
        ++it;
        height                   = (*it).toInt();
        ++it;

        record.imageSize         = QSize(width, height);

        receiver->receive(record);
    }
}

void ImageLister::listMonth(ImageListerReceiver *receiver, const QDate &date)
{
    QDate firstDayOfMonth(date.year(), date.month(), 1);
    QDate firstDayOfNextMonth = firstDayOfMonth.addMonths(1);

    QList<QVariant> values;

    {
        DatabaseAccess access;
        access.backend()->execSql(QString("SELECT DISTINCT Images.id, Images.name, Images.album, "
                                          "       Albums.albumRoot, "
                                          "       ImageInformation.rating, ImageInformation.creationDate, "
                                          "       Images.modificationDate, Images.fileSize, "
                                          "       ImageInformation.width, ImageInformation.height "
                                          " FROM Images "
                                          "       LEFT OUTER JOIN ImageInformation ON Images.id=ImageInformation.imageid "
                                          "       LEFT OUTER JOIN Albums ON Albums.id=Images.album "
                                          " WHERE ImageInformation.creationDate < ? "
                                          "   AND ImageInformation.creationDate >= ? "
                                          " ORDER BY Albums.id;"),
                                  QDateTime(firstDayOfNextMonth).toString(Qt::ISODate),
                                  QDateTime(firstDayOfMonth).toString(Qt::ISODate),
                                  &values);
    }

    int width, height;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;

        record.imageID           = (*it).toLongLong();
        ++it;
        record.name              = (*it).toString();
        ++it;
        record.albumID           = (*it).toInt();
        ++it;
        record.albumRootID       = (*it).toInt();
        ++it;
        record.rating            = (*it).toInt();
        ++it;
        record.creationDate      = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.modificationDate  = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.fileSize          = (*it).toInt();
        ++it;
        width                    = (*it).toInt();
        ++it;
        height                   = (*it).toInt();
        ++it;

        record.imageSize         = QSize(width, height);

        receiver->receive(record);
    }
}

void ImageLister::listSearch(ImageListerReceiver *receiver,
                             const QString &sqlConditionalExpression,
                             const QList<QVariant> &boundValues,
                             int limit)
{
    QList<QVariant> values;
    QString sqlQuery;
    QString errMsg;

    // query head
    sqlQuery = "SELECT DISTINCT Images.id, Images.name, Images.album, "
               "       Albums.albumRoot, "
               "       ImageInformation.rating, ImageInformation.creationDate, "
               "       Images.modificationDate, Images.fileSize, "
               "       ImageInformation.width, ImageInformation.height "
               " FROM Images "
               "       LEFT OUTER JOIN ImageInformation ON Images.id=ImageInformation.imageid "
               "       LEFT OUTER JOIN Albums ON Albums.id=Images.album "
               "WHERE ( ";

    // query body   
    sqlQuery += sqlConditionalExpression;

    // query tail
    sqlQuery += " ) ";

    if (limit > 0)
        sqlQuery += QString(" LIMIT %1; ").arg(limit);
    else
        sqlQuery += ";";

    bool executionSuccess;
    {
        DatabaseAccess access;
        executionSuccess = access.backend()->execSql(sqlQuery, boundValues, &values);
        if (!executionSuccess)
            errMsg = access.backend()->lastError();
    }

    if (!executionSuccess)
    {
        receiver->error(errMsg);
        return;
    }

    int width, height;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;

        record.imageID           = (*it).toLongLong();
        ++it;
        record.name              = (*it).toString();
        ++it;
        record.albumID           = (*it).toInt();
        ++it;
        record.albumRootID       = (*it).toInt();
        ++it;
        record.rating            = (*it).toInt();
        ++it;
        record.creationDate      = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.modificationDate  = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.fileSize          = (*it).toInt();
        ++it;
        width                    = (*it).toInt();
        ++it;
        height                   = (*it).toInt();
        ++it;

        record.imageSize         = QSize(width, height);

        receiver->receive(record);
    }
}

}  // namespace Digikam
