/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-11
 * Description : shared image loading and caching
 *
 * Copyright (C) 2005-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// Qt includes.

#include <qapplication.h>
#include <qvariant.h>

// KDE includes.

#include <kdirwatch.h>

// Local includes.

#include "ddebug.h"
#include "loadingcache.h"
#include "loadingcache.moc"

namespace Digikam
{

class LoadingCachePriv
{
public:

    QCache<DImg> imageCache;
    QDict<LoadingProcess> loadingDict;
    QMutex mutex;
    QWaitCondition condVar;
    KDirWatch *watch;
    QStringList watchedFiles;
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

    d->watch = new KDirWatch;

    connect(d->watch, SIGNAL(dirty(const QString &)),
            this, SLOT(slotFileDirty(const QString &)));
}

LoadingCache::~LoadingCache()
{
    delete d->watch;
    delete d;
    m_instance = 0;
}

DImg *LoadingCache::retrieveImage(const QString &cacheKey)
{
    return d->imageCache.find(cacheKey);
}

bool LoadingCache::putImage(const QString &cacheKey, DImg *img, const QString &filePath)
{
    bool successfulyInserted;

    // use size of image as cache cost, take care for wrapped preview QImages
    int cost = img->numBytes();
    QVariant attribute(img->attribute("previewQImage"));
    if (attribute.isValid())
    {
        cost = attribute.toImage().numBytes();
    }

    if ( d->imageCache.insert(cacheKey, img, cost) )
    {
        if (!filePath.isEmpty())
        {
            // store file path as attribute for our own use
            img->setAttribute("loadingCacheFilePath", QVariant(filePath));
        }
        successfulyInserted = true;
    }
    else
    {
        // need to delete object if it was not successfuly inserted (too large)
        delete img;
        successfulyInserted = false;
    }

    if (!filePath.isEmpty())
    {
        // schedule update of file watch
        // KDirWatch can only be accessed from main thread!
        QApplication::postEvent(this, new QCustomEvent(QEvent::User));
    }
    return successfulyInserted;
}

void LoadingCache::removeImage(const QString &cacheKey)
{
    d->imageCache.remove(cacheKey);
}

void LoadingCache::removeImages()
{
    d->imageCache.clear();
}

void LoadingCache::slotFileDirty(const QString &path)
{
    // Signal comes from main thread, we need to lock ourselves.
    CacheLock lock(this);
    //DDebug() << "LoadingCache slotFileDirty " << path << endl;
    for (QCacheIterator<DImg> it(d->imageCache); it.current(); ++it)
    {
        if (it.current()->attribute("loadingCacheFilePath").toString() == path)
        {
            //DDebug() << " removing watch and cache entry for " << path << endl;
            d->imageCache.remove(it.currentKey());
            d->watch->removeFile(path);
            d->watchedFiles.remove(path);
        }
    }
}

void LoadingCache::customEvent(QCustomEvent *)
{
    // Event comes from main thread, we need to lock ourselves.
    CacheLock lock(this);

    // get a list of files in cache that need watch
    QStringList toBeAdded;
    QStringList toBeRemoved = d->watchedFiles;
    for (QCacheIterator<DImg> it(d->imageCache); it.current(); ++it)
    {
        QString watchPath = it.current()->attribute("loadingCacheFilePath").toString();
        if (!watchPath.isEmpty())
        {
            if (!d->watchedFiles.contains(watchPath))
                toBeAdded.append(watchPath);
            toBeRemoved.remove(watchPath);
        }
    }

    for (QStringList::iterator it = toBeRemoved.begin(); it != toBeRemoved.end(); ++it)
    {
        //DDebug() << "removing watch for " << *it << endl;
        d->watch->removeFile(*it);
        d->watchedFiles.remove(*it);
    }

    for (QStringList::iterator it = toBeAdded.begin(); it != toBeAdded.end(); ++it)
    {
        //DDebug() << "adding watch for " << *it << endl;
        d->watch->addFile(*it);
        d->watchedFiles.append(*it);
    }

}

bool LoadingCache::isCacheable(const DImg *img)
{
    // return whether image fits in cache
    return (uint)d->imageCache.maxCost() >= img->numBytes();
}

void LoadingCache::addLoadingProcess(LoadingProcess *process)
{
    d->loadingDict.insert(process->cacheKey(), process);
}

LoadingProcess *LoadingCache::retrieveLoadingProcess(const QString &cacheKey)
{
    return d->loadingDict.find(cacheKey);
}

void LoadingCache::removeLoadingProcess(LoadingProcess *process)
{
    d->loadingDict.remove(process->cacheKey());
}

void LoadingCache::notifyNewLoadingProcess(LoadingProcess *process, LoadingDescription description)
{
    for (QDictIterator<LoadingProcess> it(d->loadingDict); it.current(); ++it)
    {
        it.current()->notifyNewLoadingProcess(process, description);
    }
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

