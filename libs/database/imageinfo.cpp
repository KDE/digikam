/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling accesss to one image and associated data
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

/** @file imageinfo.cpp */

// Qt includes.

#include <QFile>
#include <QFileInfo>
#include <QHash>

// Local includes.

#include "ddebug.h"
#include "albumdb.h"
#include "databaseattributeswatch.h"
#include "databaseaccess.h"
#include "imageinfodata.h"
#include "imageinfocache.h"
#include "imageinfo.h"

namespace Digikam
{

ImageInfoData::ImageInfoData()
{
    id           = -1;
    albumId      = -1;
    commentValid = false;
    rating       = -1;
    fileSize     = (uint)-1;
}

ImageInfo::ImageInfo()
    : m_data(0)
{
}

ImageInfo::ImageInfo(qlonglong Id, int albumId,
                     const QString& name,
                     const QString &albumName,
                     const KUrl& albumRoot,
                     const QDateTime& datetime,
                     uint  size,
                     const QSize& dims)
{
    DatabaseAccess access;
    m_data = access.imageInfoCache()->infoForId(Id);

    m_data->albumId        = albumId;
    m_data->url            = DatabaseUrl::fromAlbumAndName(name, albumName, albumRoot);
    m_data->dateTime       = datetime;
    m_data->fileSize       = size;
    m_data->imageDimension = dims;
}

ImageInfo::ImageInfo(const ImageListerRecord &record)
{
    DatabaseAccess access;
    m_data = access.imageInfoCache()->infoForId(record.imageID);

    m_data->albumId        = record.albumID;
    m_data->url            = DatabaseUrl::fromAlbumAndName(record.name, record.albumName, record.albumRoot);
    m_data->dateTime       = record.dateTime;
    m_data->fileSize       = record.size;
    m_data->imageDimension = record.dims;
}

ImageInfo::ImageInfo(qlonglong ID)
{
    DatabaseAccess access;
    m_data = access.imageInfoCache()->infoForId(ID);
    // retrieve immutable values now, the rest on demand
    ItemShortInfo info = access.db()->getItemShortInfo(ID);
    m_data->albumId = info.albumID;
    m_data->url     = DatabaseUrl::fromAlbumAndName(info.itemName, info.album, info.albumRoot);
}

ImageInfo::~ImageInfo()
{
    ImageInfoData *olddata = m_data.unassign();
    if (olddata)
        DatabaseAccess().imageInfoCache()->dropInfo(olddata);
}

ImageInfo::ImageInfo(const ImageInfo &info)
{
    m_data = info.m_data;
}

ImageInfo &ImageInfo::operator=(const ImageInfo &info)
{
    if (m_data == info.m_data)
        return *this;

    ImageInfoData *olddata = m_data.assign(info.m_data);
    if (olddata)
        DatabaseAccess().imageInfoCache()->dropInfo(olddata);

    return *this;
}

bool ImageInfo::isNull() const
{
    return !m_data;
}

bool ImageInfo::operator==(const ImageInfo &info) const
{
    if (m_data && info.m_data)
    {
        // not null, compare id
        return m_data->id == info.m_data->id;
    }
    else
    {
        // both null?
        return m_data == info.m_data;
    }
}

bool ImageInfo::operator<(const ImageInfo &info) const
{
    if (m_data)
    {
        if (info.m_data)
            // both not null, sort by id
            return m_data->id < info.m_data->id;
        else
            // only other is null, this is greater than
            return false;
    }
    else
    {
        // this is less than if the other is not null
        return info.m_data;
    }
}

uint ImageInfo::hash() const
{
    if (m_data)
        return ::qHash(m_data->id);
    else
        return ::qHash((int)0);
}

/**
 * Access rules for all methods in this class:
 * ImageInfoData members shall be accessed only under DatabaseAccess lock.
 * The id and albumId are the exception to this rule, as they are
 * primitive and will never change during the lifetime of an object.
 */

qlonglong ImageInfo::id() const
{
    return m_data ? m_data->id : -1;
}

int ImageInfo::albumId() const
{
    return m_data ? m_data->albumId : -1;
}

QString ImageInfo::name() const
{
    if (!m_data)
        return QString();

    DatabaseAccess access;
    return m_data->url.name();
}

uint ImageInfo::fileSize() const
{
    if (!m_data)
        return 0;

    DatabaseAccess access;
    if (m_data->fileSize == (uint)-1)
    {
        QFileInfo info(filePath());
        m_data.constCastData()->fileSize = info.size();
    }

    return m_data->fileSize;
}

QString ImageInfo::comment() const
{
    if (!m_data)
        return QString();
#warning ImageInfo: implement comment()
    DatabaseAccess access;
    if (!m_data->commentValid)
    {
        //m_data.constCastData()->comment = access.db()->getItemCaption(m_data->id);
        m_data.constCastData()->commentValid = true;
    }
    return m_data->comment;
}

int ImageInfo::rating() const
{
    if (!m_data)
        return 0;

    DatabaseAccess access;
    if (m_data->rating == -1)
        m_data.constCastData()->rating = access.db()->getItemRating(m_data->id);
    return m_data->rating;
}

QDateTime ImageInfo::dateTime() const
{
    if (!m_data)
        return QDateTime();

    DatabaseAccess access;
    if (!m_data->dateTime.isValid())
    {
        m_data.constCastData()->dateTime = access.db()->getItemDate(m_data->id);
    }
    return m_data->dateTime;
}

QDateTime ImageInfo::modDateTime() const
{
    if (!m_data)
        return QDateTime();

    DatabaseAccess access;
    if (!m_data->modDateTime.isValid())
    {
        QFileInfo fileInfo(filePath());
        m_data.constCastData()->modDateTime = fileInfo.lastModified();
    }
    return m_data->modDateTime;
}

QSize ImageInfo::dimensions() const
{
    if (!m_data)
        return QSize();

    //TODO: lazy loading
    DatabaseAccess access;
    return m_data->imageDimension;
}

DatabaseUrl ImageInfo::databaseUrl() const
{
    if (!m_data)
        return DatabaseUrl();

    DatabaseAccess access;
    return m_data->url;
}

KUrl ImageInfo::fileUrl() const
{
    if (!m_data)
        return KUrl();

    DatabaseAccess access;
    return m_data->url.fileUrl();
}


KUrl ImageInfo::kurl() const
{
    return fileUrl();
}

QString ImageInfo::filePath() const
{
    return fileUrl().path();
}

KUrl ImageInfo::kurlForKIO() const
{
    return databaseUrl();
}

void ImageInfo::setComment(const QString& caption)
{
#warning ImageInfo: implement setting of comment
    if (!m_data)
        return;

    DatabaseAccess access;
    //access.db()->setItemCaption(m_data->id, caption);
}

void ImageInfo::setRating(int value)
{
    DatabaseAccess access;
    access.db()->setItemRating(m_data->id, value);
}

void ImageInfo::setDateTime(const QDateTime& dateTime)
{
    if (!m_data)
        return;

    if (dateTime.isValid())
    {
        DatabaseAccess access;
        access.db()->setItemDate(m_data->id, dateTime);
    }
}

QStringList ImageInfo::tagNames() const
{
    if (!m_data)
        return QStringList();
    DatabaseAccess access;
    return access.db()->getItemTagNames(m_data->id);
}

QList<int> ImageInfo::tagIds() const
{
    // Cache tags?
    if (!m_data)
        return QList<int>();
    DatabaseAccess access;
    return access.db()->getItemTagIDs(m_data->id);
}

void ImageInfo::setTag(int tagID)
{
    DatabaseAccess access;
    access.db()->addItemTag(m_data->id, tagID);
}

void ImageInfo::removeTag(int tagID)
{
    DatabaseAccess access;
    access.db()->removeItemTag(m_data->id, tagID);
}

void ImageInfo::removeAllTags()
{
    DatabaseAccess access;
    access.db()->removeItemAllTags(m_data->id);
}

void ImageInfo::addTagPaths(const QStringList &tagPaths)
{
    DatabaseAccess access;
    QList<int> list = access.db()->getTagsFromTagPaths(tagPaths, false);
    for (int i=0; i<list.count(); i++)
        access.db()->addItemTag(m_data->id, list[i]);
}

ImageInfo ImageInfo::copyItem(int dstAlbumID, const QString &dstFileName)
{
    DatabaseAccess access;
    DDebug() << "ImageInfo::copyItem " << m_data->albumId << " " << m_data->url.name() << " to " << dstAlbumID << " " << dstFileName << endl;
    if (dstAlbumID == m_data->albumId && dstFileName == m_data->url.name())
        return (*this);

    int id = access.db()->copyItem(m_data->albumId, m_data->url.name(), dstAlbumID, dstFileName);

    if (id == -1)
        return ImageInfo();

    return ImageInfo(id);
}

void ImageInfo::refresh()
{
    DatabaseAccess access;

    // invalidate, load lazily
    m_data->commentValid     = false;
    m_data->rating           = -1;
    m_data->fileSize         = (uint)-1;
    m_data->dateTime         = QDateTime();
    m_data->modDateTime      = QDateTime();
    //m_data->imageDimension = QSize();
}

}  // namespace Digikam
