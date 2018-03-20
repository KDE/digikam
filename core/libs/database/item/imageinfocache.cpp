/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2013 by Michael G. Hansen <mike at mghansen dot de>
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

// Local includes

#include "coredb.h"
#include "coredbalbuminfo.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "imageinfodata.h"
#include "digikam_debug.h"

namespace Digikam
{

ImageInfoCache::ImageInfoCache()
    : m_needUpdateAlbums(true)
{
    qRegisterMetaType<ImageInfo>("ImageInfo");
    qRegisterMetaType<ImageInfoList>("ImageInfoList");
    qRegisterMetaType<QList<ImageInfo> >("QList<ImageInfo>");

    CoreDbWatch* const dbwatch = CoreDbAccess::databaseWatch();

    connect(dbwatch, SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChanged(ImageChangeset)),
            Qt::DirectConnection);

    connect(dbwatch, SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChanged(ImageTagChangeset)),
            Qt::DirectConnection);

    connect(dbwatch, SIGNAL(albumChange(AlbumChangeset)),
            this, SLOT(slotAlbumChange(AlbumChangeset)),
            Qt::DirectConnection);
}

ImageInfoCache::~ImageInfoCache()
{
}

template <class T>
DSharedDataPointer<T> toStrongRef(T* weakRef)
{
    // Called under read lock
    if (!weakRef)
    {
        return DSharedDataPointer<T>();
    }
    // The weak ref is a data object which is not deleted
    // (because deletion is done under mutex protection)
    // but may have a ref count of 0.
    // If the ref count is 0 and we gave the object away to another
    // thread, it might get deleted by this thread before the mutex
    // is acquired in the first thread which initially dropped the ref
    // count to 0 and also intends to delete it, then operating
    // on deleted data and crashing.
    // That means if the weakRef had a ref count of 0 before we incremented,
    // we need to drop it.
    int previousRef = weakRef->ref.fetchAndAddOrdered(1);

    if (previousRef == 0)
    {
        // drop weakRef
        weakRef->ref.deref();
        return DSharedDataPointer<T>();
    }

    // Convert to a strong reference. Will ref() the weakRef once again
    DSharedDataPointer<ImageInfoData> ptr(weakRef);
    // decrease counter, which we incremented twice now
    weakRef->ref.deref();
    return ptr;
}

static bool lessThanForAlbumShortInfo(const AlbumShortInfo& first, const AlbumShortInfo& second)
{
    return first.id < second.id;
}

void ImageInfoCache::checkAlbums()
{
    if (m_needUpdateAlbums)
    {
        // list comes sorted from db
        QList<AlbumShortInfo> infos = CoreDbAccess().db()->getAlbumShortInfos();
        ImageInfoWriteLocker lock;
        m_albums                    = infos;
        m_needUpdateAlbums          = false;
    }
}

DSharedDataPointer<ImageInfoData> ImageInfoCache::infoForId(qlonglong id)
{
    {
        ImageInfoReadLocker lock;
        DSharedDataPointer<ImageInfoData> ptr = toStrongRef(m_infos.value(id));

        if (ptr)
        {
            return ptr;
        }
    }

    ImageInfoWriteLocker lock;
    ImageInfoData* const data = new ImageInfoData();
    data->id                  = id;
    m_infos[id]               = data;
    return DSharedDataPointer<ImageInfoData>(data);
}

void ImageInfoCache::cacheByName(ImageInfoData* data)
{
    // Called with Write lock

    if (!data || data->id == -1 || data->name.isEmpty())
    {
        return;
    }

    // Called in a context where we can assume that the entry is not yet cached by name (newly created data)
    m_nameHash.remove(m_dataHash.value(data), data);
    m_nameHash.insert(data->name, data);
    m_dataHash.insert(data, data->name);
}

DSharedDataPointer<ImageInfoData> ImageInfoCache::infoForPath(int albumRootId, const QString& relativePath, const QString& name)
{
    ImageInfoReadLocker lock;
    // We check all entries in the multi hash with matching file name
    QMultiHash<QString, ImageInfoData*>::const_iterator it;

    for (it = m_nameHash.constFind(name); it != m_nameHash.constEnd() && it.key() == name; ++it)
    {
        // first check that album root matches
        if (it.value()->albumRootId != albumRootId)
        {
            continue;
        }

        // check that relativePath matches. We get relativePath from entry's id and compare to given name.
        QList<AlbumShortInfo>::const_iterator albumIt = findAlbum(it.value()->albumId);

        if (albumIt == m_albums.constEnd() || albumIt->relativePath != relativePath)
        {
            continue;
        }

        // we have now a match by name, albumRootId and relativePath
        return toStrongRef(it.value());
    }

    return DSharedDataPointer<ImageInfoData>();
}

void ImageInfoCache::dropInfo(ImageInfoData* infodata)
{
    if (!infodata)
    {
        return;
    }

    ImageInfoWriteLocker lock;

    m_infos.remove(infodata->id);

    m_nameHash.remove(m_dataHash.value(infodata), infodata);
    m_nameHash.remove(infodata->name, infodata);
    m_dataHash.remove(infodata);
    delete infodata;
}

QList<AlbumShortInfo>::const_iterator ImageInfoCache::findAlbum(int id)
{
    // Called with read lock
    AlbumShortInfo info;
    info.id = id;
    // we use the fact that d->infos is sorted by id
    return qBinaryFind(m_albums.constBegin(), m_albums.constEnd(), info, lessThanForAlbumShortInfo);
}

QString ImageInfoCache::albumRelativePath(int albumId)
{
    checkAlbums();
    ImageInfoReadLocker lock;
    QList<AlbumShortInfo>::const_iterator it = findAlbum(albumId);

    if (it != m_albums.constEnd())
    {
        return it->relativePath;
    }

    return QString();
}

void ImageInfoCache::invalidate()
{
    ImageInfoWriteLocker lock;
    QHash<qlonglong, ImageInfoData*>::iterator it;

    for (it = m_infos.begin(); it != m_infos.end(); ++it)
    {
        if ((*it)->isReferenced())
        {
            (*it)->invalid = true;
            (*it)->id = -1;
        }
        else
        {
            delete *it;
        }
    }

    m_infos.clear();
    m_albums.clear();
}

void ImageInfoCache::slotImageChanged(const ImageChangeset& changeset)
{
    ImageInfoWriteLocker lock;

    foreach(const qlonglong& imageId, changeset.ids())
    {
        QHash<qlonglong, ImageInfoData*>::iterator it = m_infos.find(imageId);

        if (it != m_infos.end())
        {
            // invalidate the relevant field. It will be lazy-loaded at first access.
            DatabaseFields::Set changes = changeset.changes();

            if (changes & DatabaseFields::ImageCommentsAll)
            {
                (*it)->defaultCommentCached = false;
                (*it)->defaultTitleCached   = false;
            }

            if (changes & DatabaseFields::Category)
            {
                (*it)->categoryCached = false;
            }

            if (changes & DatabaseFields::Format)
            {
                (*it)->formatCached = false;
            }

            if (changes & DatabaseFields::PickLabel)
            {
                (*it)->pickLabelCached = false;
            }

            if (changes & DatabaseFields::ColorLabel)
            {
                (*it)->colorLabelCached = false;
            }

            if (changes & DatabaseFields::Rating)
            {
                (*it)->ratingCached = false;
            }

            if (changes & DatabaseFields::CreationDate)
            {
                (*it)->creationDateCached = false;
            }

            if (changes & DatabaseFields::ModificationDate)
            {
                (*it)->modificationDateCached = false;
            }

            if (changes & DatabaseFields::FileSize)
            {
                (*it)->fileSizeCached = false;
            }

            if ((changes & DatabaseFields::Width) || (changes & DatabaseFields::Height))
            {
                (*it)->imageSizeCached = false;
            }

            if (changes & DatabaseFields::LatitudeNumber  ||
                changes & DatabaseFields::LongitudeNumber ||
                changes & DatabaseFields::Altitude)
            {
                (*it)->positionsCached = false;
            }

            if (changes & DatabaseFields::ImageRelations)
            {
                (*it)->groupedImagesCached = false;
                (*it)->groupImageCached    = false;
            }

            if (changes.hasFieldsFromVideoMetadata())
            {
                const DatabaseFields::VideoMetadata changedVideoMetadata = changes.getVideoMetadata();
                (*it)->videoMetadataCached&=~changedVideoMetadata;

                (*it)->databaseFieldsHashRaw.removeAllFields(changedVideoMetadata);
            }

            if (changes.hasFieldsFromImageMetadata())
            {
                const DatabaseFields::ImageMetadata changedImageMetadata = changes.getImageMetadata();
                (*it)->imageMetadataCached&=~changedImageMetadata;

                (*it)->databaseFieldsHashRaw.removeAllFields(changedImageMetadata);
            }
        }
    }
}

void ImageInfoCache::slotImageTagChanged(const ImageTagChangeset& changeset)
{
    if (changeset.propertiesWereChanged())
    {
        return;
    }

    ImageInfoWriteLocker lock;

    foreach(const qlonglong& imageId, changeset.ids())
    {
        QHash<qlonglong, ImageInfoData*>::iterator it = m_infos.find(imageId);

        if (it != m_infos.end())
        {
            (*it)->tagIdsCached     = false;
            (*it)->colorLabelCached = false;
            (*it)->pickLabelCached  = false;
        }
    }
}

void ImageInfoCache::slotAlbumChange(const AlbumChangeset& changeset)
{
    switch (changeset.operation())
    {
        case AlbumChangeset::Added:
        case AlbumChangeset::Deleted:
        case AlbumChangeset::Renamed:
        case AlbumChangeset::PropertiesChanged:
            m_needUpdateAlbums = true;
            break;
        case AlbumChangeset::Unknown:
            break;
    }
}

}  // namespace Digikam
