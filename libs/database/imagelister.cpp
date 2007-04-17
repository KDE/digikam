/* ============================================================
 * Authors: Marcel Wiesweg
 *          Renchi Raju
 * Date   : 2007-03-20
 * Description : database interface.
 * 
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qdir.h>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <klargefile.h>

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "dmetadata.h"
#include "namefilter.h"
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
    os << record.size;
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
    ds >> record.size;
    ds >> record.dims;
    return ds;
}

KIO::TransferJob *ImageLister::startListJob(const DatabaseUrl &url, const QString &filter,
                                            int getDimension, int extraValue)
{
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << url;
    ds << filter;
    ds << getDimension;
    if (extraValue != -1)
        ds << extraValue;

    return new KIO::TransferJob(url, KIO::CMD_SPECIAL,
                                ba, QByteArray(), false);
}

QSize ImageLister::retrieveDimension(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString rawFilesExt(raw_file_extentions);
    QString ext = fileInfo.extension(false).upper();

    if (!ext.isEmpty() && rawFilesExt.upper().contains(ext))
    {
        Digikam::DMetadata metaData(filePath);
        return metaData.getImageDimensions();
    }
    else
    {
        KFileMetaInfo metaInfo(filePath);
        if (metaInfo.isValid())
        {
            if (metaInfo.containsGroup("Jpeg EXIF Data"))
            {
                return metaInfo.group("Jpeg EXIF Data").
                         item("Dimensions").value().toSize();
            }
            else if (metaInfo.containsGroup("General"))
            {
                return metaInfo.group("General").
                         item("Dimensions").value().toSize();
            }
            else if (metaInfo.containsGroup("Technical"))
            {
                return metaInfo.group("Technical").
                         item("Dimensions").value().toSize();
            }
        }
    }
    return QSize();
}


void ImageLister::list(ImageListerReceiver *receiver, const DatabaseUrl &url,
                       const QString &filter, bool getDimension)
{
    if (url.isAlbumUrl())
    {
        QString albumRoot = url.albumRootPath();
        QString album     = url.album();
        listAlbum(receiver, albumRoot, album, filter, getDimension);
    }
    else if (url.isTagUrl())
    {
        listTag(receiver, url.tagId(), filter, getDimension);
    }
    else if (url.isDateUrl())
    {
        listMonth(receiver, url.date(), filter, getDimension);
    }
}

void ImageLister::listAlbum(ImageListerReceiver *receiver,
                            const QString &albumRoot, const QString &album,
                            const QString &filter, bool getDimension)
{
    int albumid;

    {
        DatabaseAccess access;
        albumid = access.db()->getAlbumForPath(albumRoot, album, false);

        if (albumid == -1)
            return;
    }

    listAlbum(receiver, albumRoot, album, albumid, filter, getDimension);
}

void ImageLister::listAlbum(ImageListerReceiver *receiver,
                            const QString &albumRoot, const QString &album, int albumid,
                            const QString &filter, bool getDimensions)
{
    QString base      = albumRoot + album;

    QStringList values;

    {
        DatabaseAccess access;
        access.backend()->execSql(QString("SELECT id, name, datetime FROM Images "
                                          "WHERE dirid = %1;")
                                         .arg(albumid),
                                  &values);
        /*
        With rating:
        SELECT Images.id, Images.name, Images.datetime, Albums.url, ImageProperties.value
        FROM Images
        LEFT OUTER JOIN Albums ON Images.dirid = Albums.id
        LEFT OUTER JOIN ImageProperties ON (ImageProperties.imageid = Images.id AND ImageProperties.property="rating")
        WHERE Images.dirid=1;
        */
    }

    NameFilter nameFilter(filter);
    struct stat stbuf;
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID   = (*it).toLongLong();
        ++it;
        record.name      = *it;
        ++it;
        record.dateTime  = QDateTime::fromString(*it, Qt::ISODate);
        ++it;

        record.albumID   = albumid;
        record.albumName = album;
        record.albumRoot = albumRoot;

        if (!nameFilter.matches(record.name))
            continue;

        QString filePath = base + '/' + record.name;

        if (::stat(QFile::encodeName(filePath), &stbuf) != 0)
            continue;
        record.size = static_cast<size_t>(stbuf.st_size);

        if (getDimensions)
            record.dims = retrieveDimension(filePath);

        receiver->receive(record);
    }
}

void ImageLister::listTag(ImageListerReceiver *receiver,
                          int tagId, const QString &filter, bool getDimensions)
{
    QString albumRoot = DatabaseAccess::albumRoot();

    QStringList values;

    {
        DatabaseAccess access;
        access.backend()->execSql( QString( "SELECT DISTINCT Images.id, Images.name, Images.dirid, \n "
                                            "       Images.datetime, Albums.url \n "
                                            " FROM Images, Albums \n "
                                            " WHERE Images.id IN \n "
                                            "       (SELECT imageid FROM ImageTags \n "
                                            "        WHERE tagid=%1 \n "
                                            "           OR tagid IN (SELECT id FROM TagsTree WHERE pid=%2)) \n "
                                            "   AND Albums.id=Images.dirid \n " )
                                    .arg(tagId)
                                    .arg(tagId), &values );
    }

    NameFilter nameFilter(filter);
    struct stat stbuf;
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID   = (*it).toLongLong();
        ++it;
        record.name      = *it;
        ++it;
        record.albumID   = (*it).toInt();
        ++it;
        record.dateTime  = QDateTime::fromString(*it, Qt::ISODate);
        ++it;
        record.albumName = *it;
        ++it;

        record.albumRoot = albumRoot;

        if (!nameFilter.matches(record.name))
            continue;

        QString path = albumRoot + record.albumName + '/' + record.name;

        if (::stat(QFile::encodeName(path), &stbuf) != 0)
            continue;
        record.size = static_cast<size_t>(stbuf.st_size);

        if (getDimensions)
            record.dims = retrieveDimension(path);

        receiver->receive(record);
    }
}

void ImageLister::listMonth(ImageListerReceiver *receiver,
                            const QDate &date, const QString &filter, bool getDimensions)
{
    QString albumRoot = DatabaseAccess::albumRoot();

    QString moStr1, moStr2;
    moStr1.sprintf("%.2d", date.month());
    moStr2.sprintf("%.2d", date.month()+1);

    QStringList values;

    {
        DatabaseAccess access;
        access.backend()->execSql(QString("SELECT Images.id, Images.name, Images.dirid, \n "
                                          "  Images.datetime, Albums.url \n "
                                          "FROM Images, Albums \n "
                                          "WHERE Images.datetime < '%1-%2-01' \n "
                                          "AND Images.datetime >= '%3-%4-01' \n "
                                          "AND Albums.id=Images.dirid \n "
                                          "ORDER BY Albums.id;")
                                  .arg(date.year(),4)
                                  .arg(moStr2)
                                  .arg(date.year(),4)
                                  .arg(moStr1,2),
                                  &values);
    }

    NameFilter nameFilter(filter);
    struct stat stbuf;
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID   = (*it).toLongLong();
        ++it;
        record.name      = *it;
        ++it;
        record.albumID   = (*it).toInt();
        ++it;
        record.dateTime  = QDateTime::fromString(*it, Qt::ISODate);
        ++it;
        record.albumName = *it;
        ++it;

        record.albumRoot = albumRoot;

        if (!nameFilter.matches(record.name))
            continue;

        QString path = albumRoot + record.albumName + '/' + record.name;

        if (::stat(QFile::encodeName(path), &stbuf) != 0)
            continue;
        record.size = static_cast<size_t>(stbuf.st_size);

        if (getDimensions)
            record.dims = retrieveDimension(path);

        receiver->receive(record);
    }
}

void ImageLister::listSearch(ImageListerReceiver *receiver,
                             const QString &sqlConditionalExpression, const QString &filter, bool getDimensions,
                             bool getSize, int limit)
{
    QString albumRoot = DatabaseAccess::albumRoot();

    QStringList values;
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
        executionSuccess = access.backend()->execSql(sqlQuery, &values, &errMsg);
    }

    if (!executionSuccess)
    {
        receiver->error(errMsg);
        return;
    }

    NameFilter nameFilter(filter);
    struct stat stbuf;
    for (QStringList::iterator it = values.begin(); it != values.end();)
    {
        ImageListerRecord record;
        record.imageID   = (*it).toLongLong();
        ++it;
        record.name      = *it;
        ++it;
        record.albumID   = (*it).toInt();
        ++it;
        record.dateTime  = QDateTime::fromString(*it, Qt::ISODate);
        ++it;
        record.albumName = *it;
        ++it;

        record.albumRoot = albumRoot;

        if (!nameFilter.matches(record.name))
            continue;

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

        if (getDimensions)
            record.dims = retrieveDimension(path);

        receiver->receive(record);
    }
}

}



