/* ============================================================
 * File  : imageinfo.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>

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

// Local includes.

#include "ddebug.h"
#include "albumdb.h"
#include "databaseattributeswatch.h"
#include "databaseaccess.h"
#include "imageinfo.h"

namespace Digikam
{

ImageInfo::ImageInfo()
    : m_ID(-1), m_albumID(-1), m_size(0)
{
}

ImageInfo::ImageInfo(Q_LLONG ID, int albumID,
                     const QString& name,
                     const QString &albumName,
                     const KURL& albumRoot,
                     const QDateTime& datetime, size_t size,
                     const QSize& dims)
    : m_ID(ID), m_albumID(albumID), m_datetime(datetime),
      m_size(size), m_dims(dims)
{
    m_url = DatabaseUrl::fromAlbumAndName(name, albumName, albumRoot);
}

ImageInfo::ImageInfo(const ImageListerRecord &record)
    : m_ID(record.imageID), m_albumID(record.albumID), m_datetime(record.dateTime),
      m_size(record.size), m_dims(record.dims)
{
    m_url = DatabaseUrl::fromAlbumAndName(record.name, record.albumName, record.albumRoot);
}

ImageInfo::ImageInfo(Q_LLONG ID)
    : m_ID(ID), m_size(0)
{
    // retrieve these now, the rest on demand
    DatabaseAccess access;
    m_albumID = access.db()->getItemAlbum(m_ID);
    QString name = access.db()->getItemName(m_ID);
    QString albumName = access.db()->getAlbumURL(m_albumID);
    m_url = DatabaseUrl::fromAlbumAndName(name, albumName, DatabaseAccess::albumRoot());
}

ImageInfo::~ImageInfo()
{
}

bool ImageInfo::isNull() const
{
    return m_ID != -1;
}

QString ImageInfo::name() const
{
    return m_url.name();
}

size_t ImageInfo::fileSize() const
{
    if (m_size == 0)
    {
        QFileInfo info(filePath());
        m_size = info.size();
    }
    return m_size;
}

QDateTime ImageInfo::dateTime() const
{
    if (!m_datetime.isValid())
    {
        DatabaseAccess access;
        m_datetime = access.db()->getItemDate(m_ID);
    }
    return m_datetime;
}

QDateTime ImageInfo::modDateTime() const
{
    if (!m_modDatetime.isValid())
    {
        QFileInfo fileInfo(filePath());
        m_modDatetime = fileInfo.lastModified();
    }

    return m_modDatetime;
}

QSize ImageInfo::dimensions() const
{
    //TODO: lazy loading
    return m_dims;
}

Q_LLONG ImageInfo::id() const
{
    return m_ID;
}

int ImageInfo::albumID() const
{
    return m_albumID;
}

DatabaseUrl ImageInfo::databaseUrl() const
{
    return m_url;
}

KURL ImageInfo::fileUrl() const
{
    return m_url.fileUrl();
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

void ImageInfo::setDateTime(const QDateTime& dateTime)
{
    if (dateTime.isValid())
    {
        {
            DatabaseAccess access;
            access.db()->setItemDate(m_ID, dateTime);
        }
        m_datetime = dateTime;
        DatabaseAttributesWatch::instance()->imageDateChanged(m_ID);
    }
}

void ImageInfo::setComment(const QString& caption)
{
    {
        DatabaseAccess access;
        access.db()->setItemCaption(m_ID, caption);
    }
    DatabaseAttributesWatch::instance()->imageCaptionChanged(m_ID);
}

QString ImageInfo::comment() const
{
    DatabaseAccess access;
    return access.db()->getItemCaption(m_ID);
}

QStringList ImageInfo::tagNames() const
{
    DatabaseAccess access;
    return access.db()->getItemTagNames(m_ID);
}

QValueList<int> ImageInfo::tagIDs() const
{
    DatabaseAccess access;
    return access.db()->getItemTagIDs(m_ID);
}

void ImageInfo::setTag(int tagID)
{
    {
        DatabaseAccess access;
        access.db()->addItemTag(m_ID, tagID);
    }
    DatabaseAttributesWatch::instance()->imageTagsChanged(m_ID);
}

void ImageInfo::removeTag(int tagID)
{
    {
        DatabaseAccess access;
        access.db()->removeItemTag(m_ID, tagID);
    }
    DatabaseAttributesWatch::instance()->imageTagsChanged(m_ID);
}

void ImageInfo::removeAllTags()
{
    {
        DatabaseAccess access;
        access.db()->removeItemAllTags(m_ID);
    }
    DatabaseAttributesWatch::instance()->imageTagsChanged(m_ID);
}

void ImageInfo::addTagPaths(const QStringList &tagPaths)
{
    {
        DatabaseAccess access;
        IntList list = access.db()->getTagsFromTagPaths(tagPaths);
        for (IntList::iterator it = list.begin(); it != list.end(); ++it)
            access.db()->addItemTag(m_ID, (*it));
    }
    DatabaseAttributesWatch::instance()->imageTagsChanged(m_ID);
}


int ImageInfo::rating() const
{
    DatabaseAccess access;
    return access.db()->getItemRating(m_ID);
}

void ImageInfo::setRating(int value)
{
    {
        DatabaseAccess access;
        access.db()->setItemRating(m_ID, value);
    }
    DatabaseAttributesWatch::instance()->imageRatingChanged(m_ID);
}

ImageInfo ImageInfo::copyItem(int dstAlbumID, const QString &dstFileName)
{
    DDebug() << "ImageInfo::copyItem " << m_albumID << " " << m_url.name() << " to " << dstAlbumID << " " << dstFileName << endl;
    if (dstAlbumID == m_albumID && dstFileName == m_url.name())
        return (*this);

    DatabaseAccess access;
    int id = access.db()->copyItem(m_albumID, m_url.name(), dstAlbumID, dstFileName);

    if (id == -1)
        return ImageInfo();
    ImageInfo info;
    info.m_ID      = id;
    info.m_albumID = dstAlbumID;

    QString name = access.db()->getItemName(m_ID);
    QString albumName = access.db()->getAlbumURL(m_albumID);
    info.m_url = DatabaseUrl::fromAlbumAndName(name, albumName, DatabaseAccess::albumRoot());
    // set size and datetime
    info.refresh();
    // m_dims is not set, m_viewItem is left 0
    return info;
}

void ImageInfo::refresh()
{
    // invalidate, load lazily
    m_datetime = QDateTime();

    QFileInfo fileInfo(filePath());
    m_size = fileInfo.size();
    m_modDatetime = fileInfo.lastModified();
}

}  // namespace Digikam

