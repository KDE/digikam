/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-01-11
 * Description : shared image loading and caching
 *
 * Copyright 2005 by Marcel Wiesweg
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


#include "loadingcache.h"

namespace Digikam
{

class LoadingCachePriv
{
public:

    QCache<DImg> imageCache;
    QDict<LoadingProcess> loadingDict;
    QMutex mutex;
    QWaitCondition condVar;
};


LoadingCache *LoadingCache::m_instance = 0;

LoadingCache *LoadingCache::cache()
{
    if (!m_instance)
        m_instance = new LoadingCache;
    return m_instance;
}

void LoadingCache::cleanUp()
{
    if (m_instance)
        delete m_instance;
}


LoadingCache::LoadingCache()
{
    d = new LoadingCachePriv;

    d->imageCache.setAutoDelete(true);
    // default value: 60 MB of cache
    setCacheSize(60);
}

LoadingCache::~LoadingCache()
{
    delete d;
    m_instance = 0;
}

DImg *LoadingCache::retrieveImage(const QString &filePath)
{
    return d->imageCache.find(filePath);
}

bool LoadingCache::putImage(const QString &filePath, DImg *img)
{
    // use size of image as cache cost
    if ( d->imageCache.insert(filePath, img, img->numBytes()) )
    {
        return true;
    }
    else
    {
        // need to delete object if it was not successfully inserted (too large)
        delete img;
        return false;
    }
}

void LoadingCache::removeImage(const QString &filePath)
{
    d->imageCache.remove(filePath);
}

void LoadingCache::removeImages()
{
    d->imageCache.clear();
}

bool LoadingCache::isCacheable(const DImg *img)
{
    // return whether image fits in cache
    return (uint)d->imageCache.maxCost() >= img->numBytes();
}

void LoadingCache::addLoadingProcess(LoadingProcess *process)
{
    d->loadingDict.insert(process->filePath(), process);
}

LoadingProcess *LoadingCache::retrieveLoadingProcess(const QString &filePath)
{
    return d->loadingDict.find(filePath);
}

void LoadingCache::removeLoadingProcess(LoadingProcess *process)
{
    d->loadingDict.remove(process->filePath());
}

void LoadingCache::setCacheSize(int megabytes)
{
    d->imageCache.setMaxCost(megabytes * 1024 * 1024);
}

//---------------------------------------------------------------------------------------------------

LoadingCache::CacheLock::CacheLock(LoadingCache *cache)
    : m_cache(cache)
{
    m_cache->d->mutex.lock();
}

LoadingCache::CacheLock::~CacheLock()
{
    m_cache->d->mutex.unlock();
}

void LoadingCache::CacheLock::wakeAll()
{
    // obviously the mutex is locked when this function is called
    m_cache->d->condVar.wakeAll();
}

void LoadingCache::CacheLock::timedWait()
{
    // same as above, the mutex is certainly locked
    m_cache->d->condVar.wait(&m_cache->d->mutex, 1000);
}

}   // namespace Digikam

