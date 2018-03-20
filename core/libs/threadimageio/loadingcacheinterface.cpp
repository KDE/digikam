/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-06
 * Description : shared image loading and caching
 *
 * Copyright (C) 2005-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "loadingcacheinterface.h"

// Local includes

#include "loadingcache.h"

namespace Digikam
{

void LoadingCacheInterface::initialize()
{
    LoadingCache::cache();
}

void LoadingCacheInterface::cleanUp()
{
    LoadingCache::cleanUp();
}

void LoadingCacheInterface::fileChanged(const QString& filePath)
{
    LoadingCache* cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);
    cache->notifyFileChanged(filePath);

/*  NOTE: old implementation
    QStringList possibleCacheKeys = LoadingDescription::possibleCacheKeys(filePath);
    for (QStringList::iterator it = possibleCacheKeys.begin(); it != possibleCacheKeys.end(); ++it)
    {
        cache->removeImage(*it);
    }
*/
}

void LoadingCacheInterface::connectToSignalFileChanged(QObject* object, const char* slot)
{
    LoadingCache* cache = LoadingCache::cache();
    QObject::connect(cache, SIGNAL(fileChanged(QString)),
                     object, slot,
                     Qt::QueuedConnection);
    // make it a queued connection because the signal is emitted when the CacheLock is held!
}

void LoadingCacheInterface::cleanCache()
{
    LoadingCache* cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);
    cache->removeImages();
}

void LoadingCacheInterface::cleanThumbnailCache()
{
    LoadingCache* cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);
    cache->removeThumbnails();
}

void LoadingCacheInterface::putImage(const QString& filePath, const DImg& img)
{
    LoadingCache* cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);

    if (cache->isCacheable(&img))
    {
        DImg* copy = new DImg(img);
        copy->detach();
        cache->putImage(filePath, copy, filePath);
    }
}

void LoadingCacheInterface::setCacheOptions(int cacheSize)
{
    LoadingCache* cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);
    cache->setCacheSize(cacheSize);
}

}   // namespace Digikam
