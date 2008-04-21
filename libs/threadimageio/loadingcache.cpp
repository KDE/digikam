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

#include <QEvent>
#include <QCustomEvent>
#include <QCache>
#include <QHash>

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

    LoadingCachePriv() : mutex(QMutex::Recursive)
    {
        watch = 0;
    }

    QCache<QString, DImg> imageCache;
    QCache<QString, QImage>  thumbnailImageCache;
    QCache<QString, QPixmap> thumbnailPixmapCache;
    QHash<QString, LoadingProcess *> loadingDict;
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

    setCacheSize(60);
    setThumbnailCacheSize(0, 100);

    d->watch = new KDirWatch;

    connect(d->watch, SIGNAL(dirty(const QString &)),
            this, SLOT(slotFileDirty(const QString &)));

    connect(this, SIGNAL(signalUpdateDirWatch()),
            this, SLOT(slotUpdateDirWatch()));

    // good place to call it here as LoadingCache is a singleton
    qRegisterMetaType<LoadingDescription>("LoadingDescription");
    qRegisterMetaType<DImg>("DImg");
}

LoadingCache::~LoadingCache()
{
    delete d->watch;
    delete d;
    m_instance = 0;
}

DImg *LoadingCache::retrieveImage(const QString &cacheKey)
{
    return d->imageCache[cacheKey];
}

bool LoadingCache::putImage(const QString &cacheKey, DImg *img, const QString &filePath)
{
    bool successfulyInserted;

    // use size of image as cache cost, take care for wrapped preview QImages
    int cost = img->numBytes();
    QVariant attribute(img->attribute("previewQImage"));
    if (attribute.isValid())
    {
        cost = attribute.value<QImage>().numBytes();
    }

    successfulyInserted = d->imageCache.insert(cacheKey, img, cost);

    if (successfulyInserted && !filePath.isEmpty())
    {
        // store file path as attribute for our own use
        img->setAttribute("loadingCacheFilePath", QVariant(filePath));
        // schedule update of file watch
        // KDirWatch can only be accessed from main thread!
        emit signalUpdateDirWatch();
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
    QList<QString> keys = d->imageCache.keys();
    foreach(QString cacheKey, keys)
    {
        if (d->imageCache[cacheKey]->attribute("loadingCacheFilePath").toString() == path)
        {
            //DDebug() << " removing watch and cache entry for " << path << endl;
            d->imageCache.remove(cacheKey);
            d->watch->removeFile(path);
            d->watchedFiles.removeAll(path);
        }
    }
}

void LoadingCache::slotUpdateDirWatch()
{
    // Event comes from main thread, we need to lock ourselves.
    CacheLock lock(this);

    // get a list of files in cache that need watch
    QStringList toBeAdded;
    QStringList toBeRemoved = d->watchedFiles;

    QList<QString> keys = d->imageCache.keys();
    foreach(QString cacheKey, keys)
    {
        QString watchPath = d->imageCache[cacheKey]->attribute("loadingCacheFilePath").toString();
        if (!watchPath.isEmpty())
        {
            if (!d->watchedFiles.contains(watchPath))
                toBeAdded.append(watchPath);
            toBeRemoved.removeAll(watchPath);
        }
    }

    for (QStringList::iterator it = toBeRemoved.begin(); it != toBeRemoved.end(); ++it)
    {
        //DDebug() << "removing watch for " << *it << endl;
        d->watch->removeFile(*it);
        d->watchedFiles.removeAll(*it);
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
    d->loadingDict[process->cacheKey()] = process;
}

LoadingProcess *LoadingCache::retrieveLoadingProcess(const QString &cacheKey)
{
    return d->loadingDict.value(cacheKey);
}

void LoadingCache::removeLoadingProcess(LoadingProcess *process)
{
    d->loadingDict.remove(process->cacheKey());
}

void LoadingCache::notifyNewLoadingProcess(LoadingProcess *process, LoadingDescription description)
{
    for (QHash<QString, LoadingProcess *>::const_iterator it = d->loadingDict.constBegin();
          it != d->loadingDict.constEnd(); ++it)
    {
        it.value()->notifyNewLoadingProcess(process, description);
    }
}

void LoadingCache::setCacheSize(int megabytes)
{
    d->imageCache.setMaxCost(megabytes * 1024 * 1024);
}

// --- Thumbnails ----

const QImage *LoadingCache::retrieveThumbnail(const QString &cacheKey)
{
    return d->thumbnailImageCache[cacheKey];
}

const QPixmap *LoadingCache::retrieveThumbnailPixmap(const QString &cacheKey)
{
    return d->thumbnailPixmapCache[cacheKey];
}

void LoadingCache::putThumbnail(const QString &cacheKey, const QImage &thumb)
{
    d->thumbnailImageCache.insert(cacheKey, new QImage(thumb));
}

void LoadingCache::putThumbnail(const QString &cacheKey, const QPixmap &thumb)
{
    d->thumbnailPixmapCache.insert(cacheKey, new QPixmap(thumb));
}

void LoadingCache::removeThumbnail(const QString &cacheKey)
{
    d->thumbnailImageCache.remove(cacheKey);
    d->thumbnailPixmapCache.remove(cacheKey);
}

void LoadingCache::removeThumbnails()
{
    d->thumbnailImageCache.clear();
    d->thumbnailPixmapCache.clear();
}

void LoadingCache::setThumbnailCacheSize(int numberOfQImages, int numberOfQPixmaps)
{
    d->thumbnailImageCache.setMaxCost(numberOfQImages);
    d->thumbnailPixmapCache.setMaxCost(numberOfQPixmaps);
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
