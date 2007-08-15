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
#include "dmetadata.h"
#include "imagelister.h"

namespace Digikam
{

QDataStream &operator<<(QDataStream &os, const ImageListerRecord &record)
{
    os << record.imageID;
    os << record.albumID;
    os << record.name;
    os << record.albumName;
    os << record.albumRoot;
    os << record.dateTime;
    //os << record.size;
    os << record.dims;
    return os;
}

QDataStream &operator>>(QDataStream &ds, ImageListerRecord &record)
{
    ds >> record.imageID;
    ds >> record.albumID;
    ds >> record.name;
    ds >> record.albumName;
    ds >> record.albumRoot;
    ds >> record.dateTime;
    //ds >> record.size;
    ds >> record.dims;
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
    int albumid;

    {
        DatabaseAccess access;
        albumid = access.db()->getAlbumForPath(albumRoot, album, false);

        if (albumid == -1)
            return;
    }

    listAlbum(receiver, albumRoot, album, albumid);
}

void ImageLister::listAlbum(ImageListerReceiver *receiver,
                            const QString &albumRoot, const QString &album, int albumid)
{
    QString base      = albumRoot + album;

    QList<QVariant> values;

    {
        DatabaseAccess access;
        access.backend()->execSql(QString("SELECT id, name, datetime FROM Images "
                                          "WHERE dirid = ?;"),
                                  albumid, &values);
        /*
        With rating:
        SELECT Images.id, Images.name, Images.datetime, Albums.url, ImageProperties.value
        FROM Images
        LEFT OUTER JOIN Albums ON Images.dirid = Albums.id
        LEFT OUTER JOIN ImageProperties ON (ImageProperties.imageid = Images.id AND ImageProperties.property="rating")
        WHERE Images.dirid=1;
        */
    }

    struct stat stbuf;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID   = it->toLongLong();
        ++it;
        record.name      = it->toString();
        ++it;
        record.dateTime  = QDateTime::fromString(it->toString(), Qt::ISODate);
        ++it;

        record.albumID   = albumid;
        record.albumName = album;
        record.albumRoot = albumRoot;

        QString filePath = base + '/' + record.name;

        if (::stat(QFile::encodeName(filePath), &stbuf) != 0)
            continue;
        record.size = static_cast<size_t>(stbuf.st_size);

        //if (getDimensions)
          //  record.dims = retrieveDimension(filePath);

        receiver->receive(record);
    }
}

void ImageLister::listTag(ImageListerReceiver *receiver, int tagId)
{
    QString albumRoot = DatabaseAccess::albumRoot();

    QList<QVariant> values;

    {
        DatabaseAccess access;
        access.backend()->execSql( QString( "SELECT DISTINCT Images.id, Images.name, Images.dirid, \n "
                                            "       Images.datetime, Albums.url \n "
                                            " FROM Images, Albums \n "
                                            " WHERE Images.id IN \n "
                                            "       (SELECT imageid FROM ImageTags \n "
                                            "        WHERE tagid=? \n "
                                            "           OR tagid IN (SELECT id FROM TagsTree WHERE pid=?)) \n "
                                            "   AND Albums.id=Images.dirid \n " ),
                                    tagId, tagId, &values );
    }

    struct stat stbuf;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID   = (*it).toLongLong();
        ++it;
        record.name      = (*it).toString();
        ++it;
        record.albumID   = (*it).toInt();
        ++it;
        record.dateTime  = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.albumName = (*it).toString();
        ++it;

        record.albumRoot = albumRoot;

        QString path = albumRoot + record.albumName + '/' + record.name;

        if (::stat(QFile::encodeName(path), &stbuf) != 0)
            continue;
        record.size = static_cast<size_t>(stbuf.st_size);

        //if (getDimensions)
          //  record.dims = retrieveDimension(path);

        receiver->receive(record);
    }
}

void ImageLister::listMonth(ImageListerReceiver *receiver, const QDate &date)
{
    QString albumRoot = DatabaseAccess::albumRoot();

    QDate firstDayOfMonth(date.year(), date.month(), 1);
    QDate firstDayOfNextMonth = firstDayOfMonth.addMonths(1);

    QList<QVariant> values;

    {
        DatabaseAccess access;
        access.backend()->execSql(QString("SELECT Images.id, Images.name, Images.dirid, \n "
                                          "  Images.datetime, Albums.url \n "
                                          "FROM Images, Albums \n "
                                          "WHERE Images.datetime < ? \n "
                                          "AND Images.datetime >= ? \n "
                                          "AND Albums.id=Images.dirid \n "
                                          "ORDER BY Albums.id;"),
                                  QDateTime(firstDayOfNextMonth).toString(Qt::ISODate),
                                  QDateTime(firstDayOfMonth).toString(Qt::ISODate),
                                  &values);
    }

    struct stat stbuf;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID   = (*it).toLongLong();
        ++it;
        record.name      = (*it).toString();
        ++it;
        record.albumID   = (*it).toInt();
        ++it;
        record.dateTime  = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.albumName = (*it).toString();
        ++it;

        record.albumRoot = albumRoot;

        QString path = albumRoot + record.albumName + '/' + record.name;

        if (::stat(QFile::encodeName(path), &stbuf) != 0)
            continue;
        record.size = static_cast<size_t>(stbuf.st_size);

        //if (getDimensions)
          //  record.dims = retrieveDimension(path);

        receiver->receive(record);
    }
}

void ImageLister::listSearch(ImageListerReceiver *receiver,
                             const QString &sqlConditionalExpression,
                             const QList<QVariant> &boundValues,
                             bool getSize, int limit)
{
    QString albumRoot = DatabaseAccess::albumRoot();

    QList<QVariant> values;
    QString sqlQuery;
    QString errMsg;

    // query head
    sqlQuery = "SELECT Images.id, Images.name, Images.dirid, Images.datetime, Albums.url "
                "FROM Images, Albums LEFT JOIN ImageProperties ON Images.id = Imageproperties.imageid "
                "WHERE ( ";

    // query body
    sqlQuery += sqlConditionalExpression;

    // query tail
    sqlQuery += " ) ";
    sqlQuery += " AND (Albums.id=Images.dirid) ";

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

    struct stat stbuf;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID   = (*it).toLongLong();
        ++it;
        record.name      = (*it).toString();
        ++it;
        record.albumID   = (*it).toInt();
        ++it;
        record.dateTime  = QDateTime::fromString((*it).toString(), Qt::ISODate);
        ++it;
        record.albumName = (*it).toString();
        ++it;

        record.albumRoot = albumRoot;

        QString path = albumRoot + record.albumName + '/' + record.name;

        if (getSize)
        {
            if (::stat(QFile::encodeName(path), &stbuf) != 0)
                continue;
            record.size = static_cast<size_t>(stbuf.st_size);
        }
        else
        {
            record.size = 0;
        }

        //if (getDimensions)
          //  record.dims = retrieveDimension(path);

        receiver->receive(record);
    }
}

}  // namespace Digikam
