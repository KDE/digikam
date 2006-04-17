/* ============================================================
 * File  : imageinfo.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "dio.h"
#include "imageinfo.h"
#include "imageattributeswatch.h"

namespace Digikam
{

AlbumManager* ImageInfo::m_man = 0;

ImageInfo::ImageInfo()
    : m_ID(-1), m_albumID(-1), m_size(0), m_viewitem(0)
{
}

ImageInfo::ImageInfo(Q_LLONG ID, int albumID, const QString& name,
                     const QDateTime& datetime, size_t size,
                     const QSize& dims)
    : m_ID(ID), m_albumID(albumID), m_name(name), m_datetime(datetime),
      m_size(size), m_dims(dims), m_viewitem(0)
{
    if (!m_man)
    {
        m_man = AlbumManager::instance();
    }
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
    return m_name;
}

bool ImageInfo::setName(const QString& newName)
{
    KURL src = kurlForKIO();
    KURL dst = src.upURL();
    dst.addPath(newName);

    if (!DIO::renameFile(src, dst))
        return false;

    PAlbum* a = album();
    if (!a)
    {
        kdWarning() << "No album found for ID: " << m_albumID << endl;
        return false;
    }
    
    m_name = newName;
    return true;
}

size_t ImageInfo::fileSize() const
{
    return m_size;    
}

QDateTime ImageInfo::dateTime() const
{
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

PAlbum* ImageInfo::album() const
{
    return m_man->findPAlbum(m_albumID);
}

KURL ImageInfo::kurl() const
{
    PAlbum* a = album();
    if (!a)
    {
        kdWarning() << "No album found for ID: " << m_albumID << endl;
        return KURL();
    }
    
    KURL u(m_man->getLibraryPath());
    u.addPath(a->url());
    u.addPath(m_name);
    return u;
}

QString ImageInfo::filePath() const
{
    PAlbum* a = album();
    if (!a)
    {
        kdWarning() << "No album found for ID: " << m_albumID << endl;
        return QString();
    }

    QString path = m_man->getLibraryPath();
    path += a->url() + "/" + m_name;
    return path;
}

KURL ImageInfo::kurlForKIO() const
{
    PAlbum* a = album();
    if (!a)
    {
        kdWarning() << "No album found for ID: " << m_albumID << endl;
        return KURL();
    }

    KURL u(a->kurl());
    u.addPath(m_name);
    return u;
}

void ImageInfo::setViewItem(void *d)
{
    m_viewitem = d;
}

void* ImageInfo::getViewItem() const
{
    return m_viewitem;
}

void ImageInfo::setDateTime(const QDateTime& dateTime)
{
    AlbumDB* db  = m_man->albumDB();
    db->setItemDate(m_ID, dateTime);
    m_datetime = dateTime;
    ImageAttributesWatch::instance()->imageDateChanged(m_ID);
}

void ImageInfo::setCaption(const QString& caption)
{
    AlbumDB* db  = m_man->albumDB();
    return db->setItemCaption(m_ID, caption);
    ImageAttributesWatch::instance()->imageCaptionChanged(m_ID);
}

QString ImageInfo::caption() const
{
    AlbumDB* db  = m_man->albumDB();
    return db->getItemCaption(m_ID);
}

QStringList ImageInfo::tagNames() const
{
    AlbumDB* db  = m_man->albumDB();
    return db->getItemTagNames(m_ID);
}

QStringList ImageInfo::tagPaths() const
{
    QStringList tagPaths;
    
    AlbumDB* db  = m_man->albumDB();
    IntList tagIDs = db->getItemTagIDs(m_ID);
    for (IntList::iterator it = tagIDs.begin(); it != tagIDs.end(); ++it)
    {
        TAlbum* ta = m_man->findTAlbum(*it);
        if (ta)
        {
            tagPaths.append(ta->url());
        }
    }

    return tagPaths;
}

QValueList<int> ImageInfo::tagIDs() const
{
    AlbumDB* db  = m_man->albumDB();
    return db->getItemTagIDs(m_ID);
}

void ImageInfo::setTag(int tagID)
{
    AlbumDB* db  = m_man->albumDB();
    db->addItemTag(m_ID, tagID);
    ImageAttributesWatch::instance()->imageTagsChanged(m_ID);
}

void ImageInfo::removeTag(int tagID)
{
    AlbumDB* db  = m_man->albumDB();
    db->removeItemTag(m_ID, tagID);
    ImageAttributesWatch::instance()->imageTagsChanged(m_ID);
}

void ImageInfo::removeAllTags()
{
    AlbumDB *db = m_man->albumDB();
    db->removeItemAllTags(m_ID);
    ImageAttributesWatch::instance()->imageTagsChanged(m_ID);
}

int ImageInfo::rating() const
{
    AlbumDB* db  = m_man->albumDB();
    return db->getItemRating(m_ID);
}

void ImageInfo::setRating(int value)
{
    AlbumDB* db  = m_man->albumDB();
    db->setItemRating(m_ID, value);
    ImageAttributesWatch::instance()->imageRatingChanged(m_ID);
}

ImageInfo ImageInfo::copyItem(PAlbum *dstAlbum, const QString &dstFileName)
{
    kdDebug() << "ImageInfo::copyItem " << m_albumID << " " << m_name << " to " << dstAlbum->id() << " " << dstFileName << endl;
    if (dstAlbum->id() == m_albumID && dstFileName == m_name)
        return (*this);

    AlbumDB* db = m_man->albumDB();
    int id = db->copyItem(m_albumID, m_name, dstAlbum->id(), dstFileName);

    if (id == -1)
        return ImageInfo();
    ImageInfo info;
    info.m_ID      = id;
    info.m_albumID = dstAlbum->id();
    info.m_name    = dstFileName;
    // set size and datetime
    info.refresh();
    // m_dims is not set, m_viewItem is left 0
    return info;
}

void ImageInfo::refresh()
{
    m_datetime = m_man->albumDB()->getItemDate(m_ID);

    QFileInfo fileInfo(filePath());
    m_size = fileInfo.size();
    m_modDatetime = fileInfo.lastModified();
}

}  // namespace Digikam

