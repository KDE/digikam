/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imageinfocache.h"
#include "imageinfocache.moc"

// Local includes

#include "albumdb.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "imageinfodata.h"

namespace Digikam
{

ImageInfoCache::ImageInfoCache()
{
    qRegisterMetaType<ImageInfo>("ImageInfo");
    qRegisterMetaType<ImageInfoList>("ImageInfoList");
    qRegisterMetaType<QList<ImageInfo> >("QList<ImageInfo>");

    DatabaseWatch *dbwatch = DatabaseAccess::databaseWatch();

    connect(dbwatch, SIGNAL(imageChange(const ImageChangeset &)),
            this, SLOT(slotImageChanged(const ImageChangeset &)));

    connect(dbwatch, SIGNAL(imageTagChange(const ImageTagChangeset &)),
            this, SLOT(slotImageTagChanged(const ImageTagChangeset &)));

    connect(dbwatch, SIGNAL(albumChange(const AlbumChangeset &)),
            this, SLOT(slotAlbumChange(const AlbumChangeset &)));
}

ImageInfoCache::~ImageInfoCache()
{
}

ImageInfoData *ImageInfoCache::infoForId(qlonglong id)
{
    QHash<qlonglong, ImageInfoData *>::iterator it = m_infos.find(id);
    if (it == m_infos.end())
    {
        ImageInfoData *data = new ImageInfoData();
        data->id            = id;
        m_infos[id]         = data;
        return data;
    }
    return (*it);
}

bool ImageInfoCache::hasInfoForId(qlonglong id) const
{
    return m_infos.contains(id);
}

void ImageInfoCache::dropInfo(ImageInfoData *infodata)
{
    // check again ref count, now in mutex-protected context
    if (infodata->isReferenced())
        return;

    if (m_infos.remove(infodata->id))
        delete infodata;
}

QString ImageInfoCache::albumName(DatabaseAccess& access, int albumId)
{
    QHash<int, QString>::iterator it = m_albums.find(albumId);
    if (it == m_albums.end())
    {
        QString album = access.db()->getAlbumRelativePath(albumId);
        m_albums[albumId] = album;
        return album;
    }
    return (*it);
}

void ImageInfoCache::slotImageChanged(const ImageChangeset& changeset)
{
    // we cannot know if we have databaseaccess lock here as well
    DatabaseAccess access;

    foreach (const qlonglong& imageId, changeset.ids())
    {
        QHash<qlonglong, ImageInfoData *>::iterator it = m_infos.find(imageId);
        if (it != m_infos.end())
        {
            // invalidate the relevant field. It will be lazy-loaded at first access.
            DatabaseFields::Set changes = changeset.changes();
            if (changes & DatabaseFields::ImageCommentsAll)
                (*it)->defaultCommentCached = false;
            if (changes & DatabaseFields::Category)
                (*it)->categoryCached = false;
            if (changes & DatabaseFields::Format)
                (*it)->formatCached = false;
            if (changes & DatabaseFields::Rating)
                (*it)->ratingCached = false;
            if (changes & DatabaseFields::CreationDate)
                (*it)->creationDateCached = false;
            if (changes & DatabaseFields::ModificationDate)
                (*it)->modificationDateCached = false;
            if (changes & DatabaseFields::FileSize)
                (*it)->fileSizeCached = false;
            if (changes & DatabaseFields::Width || changes & DatabaseFields::Height)
                (*it)->imageSizeCached = false;
        }
    }
}

void ImageInfoCache::slotImageTagChanged(const ImageTagChangeset& changeset)
{
    DatabaseAccess access;

    foreach (const qlonglong& imageId, changeset.ids())
    {
        QHash<qlonglong, ImageInfoData *>::iterator it = m_infos.find(imageId);
        if (it != m_infos.end())
            (*it)->tagIdsCached = false;
    }
}

void ImageInfoCache::slotAlbumChange(const AlbumChangeset& changeset)
{
    DatabaseAccess access;

    switch(changeset.operation())
    {
        case AlbumChangeset::Added:
        case AlbumChangeset::Deleted:
        case AlbumChangeset::Renamed:
        case AlbumChangeset::PropertiesChanged:
            m_albums.remove(changeset.albumId());
            break;
        case AlbumChangeset::Unknown:
            break;
    }
}

}  // namespace Digikam
