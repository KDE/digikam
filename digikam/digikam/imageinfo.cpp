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

#include <qfile.h>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumfilecopymove.h"
#include "imageinfo.h"

AlbumManager* ImageInfo::m_man = 0;

ImageInfo::ImageInfo(int albumID, const QString& name,
                     const QDateTime& datetime, size_t size,
                     const QSize& dims)
    : m_albumID(albumID), m_name(name), m_datetime(datetime),
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

QString ImageInfo::name() const
{
    return m_name;
}

bool ImageInfo::setName(const QString& newName)
{
    PAlbum* a = album();

    if (!AlbumFileCopyMove::rename(a, m_name, newName))
        return false;


    if (a->getIcon() == m_name)
    {
        QString err;
        AlbumManager::instance()->updatePAlbumIcon( a, newName,
                                                    false, err );
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

QSize ImageInfo::dimensions() const
{
    return m_dims;
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
    KURL u(m_man->getLibraryPath());
    u.addPath(album()->getURL());
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

QString ImageInfo::caption() const
{
    PAlbum* a = album();

    AlbumDB* db  = m_man->albumDB();
    return db->getItemCaption(a, m_name);
}

QStringList ImageInfo::tagNames() const
{
    PAlbum* a = album();

    AlbumDB* db  = m_man->albumDB();
    return db->getItemTagNames(a, m_name);
}

QStringList ImageInfo::tagPaths() const
{
    PAlbum* a = album();

    QStringList tagPaths;
    
    AlbumDB* db  = m_man->albumDB();
    IntList tagIDs = db->getItemTagIDs(a, m_name);
    for (IntList::iterator it = tagIDs.begin(); it != tagIDs.end(); ++it)
    {
        TAlbum* ta = m_man->findTAlbum(*it);
        if (ta)
        {
            tagPaths.append(ta->getURL());
        }
    }

    return tagPaths;
}

QValueList<int> ImageInfo::tagIDs() const
{
    PAlbum* a = album();

    AlbumDB* db  = m_man->albumDB();
    return db->getItemTagIDs(a, m_name);
}

void ImageInfo::setTag(int tagID)
{
    PAlbum* pa = album();
    TAlbum* ta = m_man->findTAlbum(tagID);

    AlbumDB* db  = m_man->albumDB();
    db->setItemTag(pa, m_name, ta);
}

void ImageInfo::removeTag(int tagID)
{
    PAlbum* pa = album();
    TAlbum* ta = m_man->findTAlbum(tagID);

    AlbumDB* db  = m_man->albumDB();
    db->removeItemTag(pa, m_name, ta);
}

void ImageInfo::refresh()
{
    m_datetime = m_man->albumDB()->getItemDate(album(), m_name);

    struct stat stbuf;
    stat(QFile::encodeName(kurl().path()), &stbuf);
    m_size = stbuf.st_size;
}
