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

#include <qfile.h>
#include <qfileinfo.h>
//Added by qt3to4:
#include <Q3ValueList>

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
                     const KURL& albumRoot,
                     const QDateTime& datetime,
                     uint  size,
                     const QSize& dims)
{
    DatabaseAccess access;
    m_data = access.imageInfoCache()->infoForId(Id);
    m_data->ref();

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
    m_data->ref();

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
    m_data->deref();
    if (m_data->count == 1)
        DatabaseAccess().imageInfoCache()->dropInfo(m_data);
}

ImageInfo::ImageInfo(const ImageInfo &info)
{
    m_data = info.m_data;
    if (m_data)
        m_data->ref();
}

ImageInfo &ImageInfo::operator=(const ImageInfo &info)
{
    if (m_data == info.m_data)
        return *this;

    if (m_data)
    {
        m_data->deref();
        if (m_data->count == 1)
            DatabaseAccess().imageInfoCache()->dropInfo(m_data);
    }

    m_data = info.m_data;
    if (m_data)
        m_data->ref();

    return *this;
}

bool ImageInfo::isNull() const
{
    return !m_data;
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
        m_data->fileSize = info.size();
    }

    return m_data->fileSize;
}

QString ImageInfo::comment() const
{
    if (!m_data)
        return QString();

    DatabaseAccess access;
    if (!m_data->commentValid)
    {
        m_data->comment = access.db()->getItemCaption(m_data->id);
        m_data->commentValid = true;
    }
    return m_data->comment;
}

int ImageInfo::rating() const
{
    if (!m_data)
        return 0;

    DatabaseAccess access;
    if (m_data->rating == -1)
        m_data->rating = access.db()->getItemRating(m_data->id);
    return m_data->rating;
}

QDateTime ImageInfo::dateTime() const
{
    if (!m_data)
        return QDateTime();

    DatabaseAccess access;
    if (!m_data->dateTime.isValid())
    {
        m_data->dateTime = access.db()->getItemDate(m_data->id);
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
        m_data->modDateTime = fileInfo.lastModified();
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

KURL ImageInfo::fileUrl() const
{
    if (!m_data)
        return KURL();

    DatabaseAccess access;
    return m_data->url.fileUrl();
}


KURL ImageInfo::kurl() const
{
    return fileUrl();
}

QString ImageInfo::filePath() const
{
    return fileUrl().path();
}

KURL ImageInfo::kurlForKIO() const
{
    return databaseUrl();
}

void ImageInfo::setComment(const QString& caption)
{
    if (!m_data)
        return;

    DatabaseAccess access;
    access.db()->setItemCaption(m_data->id, caption);
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

Q3ValueList<int> ImageInfo::tagIds() const
{
    // Cache tags?
    if (!m_data)
        return Q3ValueList<int>();
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
    IntList list = access.db()->getTagsFromTagPaths(tagPaths);
    for (IntList::iterator it = list.begin(); it != list.end(); ++it)
        access.db()->addItemTag(m_data->id, (*it));
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

