/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ItemInfo common data
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2014-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iteminfocache.h"

// Local includes

#include "coredb.h"
#include "coredbalbuminfo.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "iteminfodata.h"
#include "digikam_debug.h"

namespace Digikam
{

ItemInfoCache::ItemInfoCache()
    : m_needUpdateAlbums(true),
      m_needUpdateGrouped(true)
{
    qRegisterMetaType<ItemInfo>("ItemInfo");
    qRegisterMetaType<ItemInfoList>("ItemInfoList");
    qRegisterMetaType<QList<ItemInfo> >("QList<ItemInfo>");

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

ItemInfoCache::~ItemInfoCache()
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
    DSharedDataPointer<ItemInfoData> ptr(weakRef);
    // decrease counter, which we incremented twice now
    weakRef->ref.deref();

    return ptr;
}

static bool lessThanForAlbumShortInfo(const AlbumShortInfo& first, const AlbumShortInfo& second)
{
    return first.id < second.id;
}

void ItemInfoCache::checkAlbums()
{
    if (m_needUpdateAlbums)
    {
        // list comes sorted from db
        QList<AlbumShortInfo> infos = CoreDbAccess().db()->getAlbumShortInfos();

        ItemInfoWriteLocker lock;
        m_albums                    = infos;
        m_needUpdateAlbums          = false;
    }
}

int ItemInfoCache::getImageGroupedCount(qlonglong id)
{
    if (m_needUpdateGrouped)
    {
        QList<qlonglong> ids = CoreDbAccess().db()->getRelatedImagesToByType(DatabaseRelation::Grouped);

        ItemInfoWriteLocker lock;
        m_grouped            = ids;
        m_needUpdateGrouped  = false;
    }

    ItemInfoReadLocker lock;
    return m_grouped.count(id);
}

DSharedDataPointer<ItemInfoData> ItemInfoCache::infoForId(qlonglong id)
{
    {
        ItemInfoReadLocker lock;
        DSharedDataPointer<ItemInfoData> ptr = toStrongRef(m_infos.value(id));

        if (ptr)
        {
            return ptr;
        }
    }

    ItemInfoWriteLocker lock;
    ItemInfoData* const data = new ItemInfoData();
    data->id                  = id;
    m_infos[id]               = data;

    return DSharedDataPointer<ItemInfoData>(data);
}

void ItemInfoCache::cacheByName(ItemInfoData* const data)
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

DSharedDataPointer<ItemInfoData> ItemInfoCache::infoForPath(int albumRootId, const QString& relativePath, const QString& name)
{
    ItemInfoReadLocker lock;
    // We check all entries in the multi hash with matching file name
    QMultiHash<QString, ItemInfoData*>::const_iterator it;

    for (it = m_nameHash.constFind(name) ; it != m_nameHash.constEnd() && it.key() == name ; ++it)
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

    return DSharedDataPointer<ItemInfoData>();
}

void ItemInfoCache::dropInfo(ItemInfoData* const infodata)
{
    if (!infodata)
    {
        return;
    }

    ItemInfoWriteLocker lock;

    m_infos.remove(infodata->id);

    m_nameHash.remove(m_dataHash.value(infodata), infodata);
    m_nameHash.remove(infodata->name, infodata);
    m_dataHash.remove(infodata);
    delete infodata;
}

QList<AlbumShortInfo>::const_iterator ItemInfoCache::findAlbum(int id)
{
    // Called with read lock
    AlbumShortInfo info;
    info.id = id;
    // we use the fact that d->infos is sorted by id
    QList<AlbumShortInfo>::const_iterator it;
    it = std::lower_bound(m_albums.constBegin(),
                          m_albums.constEnd(), info,
                          lessThanForAlbumShortInfo);

    if (it == m_albums.constEnd() || info.id < (*it).id)
    {
        return m_albums.constEnd();
    }

    return it;
}

QString ItemInfoCache::albumRelativePath(int albumId)
{
    checkAlbums();
    ItemInfoReadLocker lock;
    QList<AlbumShortInfo>::const_iterator it = findAlbum(albumId);

    if (it != m_albums.constEnd())
    {
        return it->relativePath;
    }

    return QString();
}

void ItemInfoCache::invalidate()
{
    ItemInfoWriteLocker lock;
    QHash<qlonglong, ItemInfoData*>::iterator it;

    for (it = m_infos.begin() ; it != m_infos.end() ; ++it)
    {
        if ((*it)->isReferenced())
        {
            (*it)->invalid = true;
            (*it)->id      = -1;
        }
        else
        {
            delete *it;
        }
    }

    m_infos.clear();
    m_albums.clear();
    m_grouped.clear();
    m_nameHash.clear();
    m_dataHash.clear();
    m_needUpdateAlbums  = true;
    m_needUpdateGrouped = true;
}

void ItemInfoCache::slotImageChanged(const ImageChangeset& changeset)
{
    ItemInfoWriteLocker lock;

    foreach (const qlonglong& imageId, changeset.ids())
    {
        QHash<qlonglong, ItemInfoData*>::iterator it = m_infos.find(imageId);

        if (it != m_infos.end())
        {
            // invalidate the relevant field. It will be lazy-loaded at first access.
            DatabaseFields::Set changes = changeset.changes();

            if (changes & DatabaseFields::ItemCommentsAll)
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

            if (changes & DatabaseFields::ManualOrder)
            {
                (*it)->manualOrderCached = false;
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
                (*it)->groupImageCached = false;
                m_needUpdateGrouped     = true;
            }

            if (changes.hasFieldsFromVideoMetadata())
            {
                const DatabaseFields::VideoMetadata changedVideoMetadata = changes.getVideoMetadata();
                (*it)->videoMetadataCached                              &= ~changedVideoMetadata;
                (*it)->hasVideoMetadata                                  = true;

                (*it)->databaseFieldsHashRaw.removeAllFields(changedVideoMetadata);
            }

            if (changes.hasFieldsFromImageMetadata())
            {
                const DatabaseFields::ImageMetadata changedImageMetadata = changes.getImageMetadata();
                (*it)->imageMetadataCached                              &= ~changedImageMetadata;
                (*it)->hasImageMetadata                                  = true;

                (*it)->databaseFieldsHashRaw.removeAllFields(changedImageMetadata);
            }
        }
        else
        {
            m_needUpdateGrouped = true;
        }
    }
}

void ItemInfoCache::slotImageTagChanged(const ImageTagChangeset& changeset)
{
    if (changeset.propertiesWereChanged())
    {
        return;
    }

    ItemInfoWriteLocker lock;

    foreach (const qlonglong& imageId, changeset.ids())
    {
        QHash<qlonglong, ItemInfoData*>::iterator it = m_infos.find(imageId);

        if (it != m_infos.end())
        {
            (*it)->tagIdsCached     = false;
            (*it)->colorLabelCached = false;
            (*it)->pickLabelCached  = false;
        }
    }
}

void ItemInfoCache::slotAlbumChange(const AlbumChangeset& changeset)
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

} // namespace Digikam
