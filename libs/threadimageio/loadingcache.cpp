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


#include "loadingcache.h"

// Qt includes.

#include <QEvent>
#include <QCustomEvent>
#include <QCache>
#include <QHash>

// KDE includes.

#include <kdebug.h>
#include <kdirwatch.h>


namespace Digikam
{

class LoadingCachePriv
{
public:

    LoadingCachePriv(LoadingCache *q) : q(q)
    {
        // Note: Don't make the mutex recursive, we need to use a wait condition on it
        watch = 0;
    }

    QCache<QString, DImg> imageCache;
    QCache<QString, QImage>  thumbnailImageCache;
    QCache<QString, QPixmap> thumbnailPixmapCache;
    QMultiHash<QString, QString> filePathToCacheKeyHash;
    QHash<QString, LoadingProcess *> loadingDict;
    QMutex mutex;
    QWaitCondition condVar;
    LoadingCacheFileWatch *watch;

    void addToWatchAndHash(const QString &filePath, const QString &cacheKey);
    void removeFilePath(const QString &filePath);
    void mapFilePathToCacheKey(const QString &filePath, const QString &cacheKey);
    void cleanUpFilePathToCacheKeyHash();

    LoadingCache *q;
};

class ClassicLoadingCacheFileWatch : public QObject, public LoadingCacheFileWatch
{
    Q_OBJECT

public:

    ClassicLoadingCacheFileWatch();
    ~ClassicLoadingCacheFileWatch();
    virtual void addedImage(const QString &filePath);

private slots:

    void slotFileDirty(const QString &path);
    void slotUpdateDirWatch();

signals:

    void signalUpdateDirWatch();

private:

    KDirWatch  *watch;
    QStringList watchedFiles;
};

// for ClassicLoadingCacheFileWatch
#include "loadingcache.moc"


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
    d = new LoadingCachePriv(this);

    setCacheSize(60);
    setThumbnailCacheSize(0, 100);

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
        d->addToWatchAndHash(filePath, cacheKey);

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

void LoadingCachePriv::removeFilePath(const QString &filePath)
{
    QList<QString> keys = filePathToCacheKeyHash.values(filePath);
    foreach(const QString &cacheKey, keys)
    {
        imageCache.remove(cacheKey);
        thumbnailImageCache.remove(cacheKey);
        thumbnailPixmapCache.remove(cacheKey);
    }
}

void LoadingCachePriv::addToWatchAndHash(const QString &filePath, const QString &cacheKey)
{
    mapFilePathToCacheKey(filePath, cacheKey);

    // install default watch if no watch is set yet
    if (!watch)
        q->setFileWatch(new ClassicLoadingCacheFileWatch);

    // notify watch
    watch->addedImage(filePath);
}

void LoadingCachePriv::mapFilePathToCacheKey(const QString &filePath, const QString &cacheKey)
{
    if (filePathToCacheKeyHash.size() > 5*(imageCache.size() + thumbnailImageCache.size() + thumbnailPixmapCache.size()))
        cleanUpFilePathToCacheKeyHash();

    filePathToCacheKeyHash.insert(filePath, cacheKey);
}

void LoadingCachePriv::cleanUpFilePathToCacheKeyHash()
{
    QSet<QString> keys;
    keys += imageCache.keys().toSet();
    keys += thumbnailImageCache.keys().toSet();
    keys += thumbnailPixmapCache.keys().toSet();
    QMultiHash<QString, QString>::iterator it;
    for (it = filePathToCacheKeyHash.begin(); it != filePathToCacheKeyHash.end(); )
    {
        if (!keys.contains(it.value()))
            it = filePathToCacheKeyHash.erase(it);
        else
            ++it;
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

void LoadingCache::putThumbnail(const QString &cacheKey, const QImage &thumb, const QString &filePath)
{
    int cost = thumb.numBytes();
    if (d->thumbnailImageCache.insert(cacheKey, new QImage(thumb), cost))
        d->addToWatchAndHash(filePath, cacheKey);

}

void LoadingCache::putThumbnail(const QString &cacheKey, const QPixmap &thumb, const QString &filePath)
{
    int cost = thumb.width() * thumb.height() * thumb.depth() / 8;
    if (d->thumbnailPixmapCache.insert(cacheKey, new QPixmap(thumb), cost))
        d->addToWatchAndHash(filePath, cacheKey);
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
    d->thumbnailImageCache.setMaxCost(numberOfQImages * 256 * 256 * 4);
    d->thumbnailPixmapCache.setMaxCost(numberOfQPixmaps * 256 * 256 * QPixmap::defaultDepth() / 8);
}

void LoadingCache::setFileWatch(LoadingCacheFileWatch *watch)
{
    delete d->watch;
    d->watch = watch;
    d->watch->m_cache = this;

    foreach(const QString &filePath, filePathsInCache())
        d->watch->addedImage(filePath);
}

QStringList LoadingCache::filePathsInCache() const
{
    d->cleanUpFilePathToCacheKeyHash();
    return d->filePathToCacheKeyHash.uniqueKeys();
}


//---------------------------------------------------------------------------------------------------

LoadingCacheFileWatch::~LoadingCacheFileWatch()
{
    if (m_cache)
    {
        LoadingCache::CacheLock lock(m_cache);
        m_cache->d->watch = 0;
    }
}

void LoadingCacheFileWatch::removeFromCache(const QString &filePath)
{
    if (m_cache)
        m_cache->d->removeFilePath(filePath);
}

void LoadingCacheFileWatch::addedImage(const QString &)
{
    // default: do nothing
}

//---------------------------------------------------------------------------------------------------

ClassicLoadingCacheFileWatch::ClassicLoadingCacheFileWatch()
{
    watch = new KDirWatch;

    connect(watch, SIGNAL(dirty(const QString &)),
            this, SLOT(slotFileDirty(const QString &)));

    // Make sure the signal gets here directly from the event loop.
    // If putImage is called from the main thread, with CacheLock,
    // a deadlock would result (mutex is not recursive)
    connect(this, SIGNAL(signalUpdateDirWatch()),
            this, SLOT(slotUpdateDirWatch()),
            Qt::QueuedConnection);

}

ClassicLoadingCacheFileWatch::~ClassicLoadingCacheFileWatch()
{
    delete watch;
}

void ClassicLoadingCacheFileWatch::addedImage(const QString &filePath)
{
    Q_UNUSED(filePath)
    // schedule update of file watch
    // KDirWatch can only be accessed from main thread!
    emit signalUpdateDirWatch();
}

void ClassicLoadingCacheFileWatch::slotFileDirty(const QString &path)
{
    // Signal comes from main thread, we need to lock ourselves.
    LoadingCache::CacheLock lock(m_cache);
    //kDebug(50003) << "LoadingCache slotFileDirty " << path << endl;
    removeFromCache(path);
    watch->removeFile(path);
    watchedFiles.removeAll(path);
}

void ClassicLoadingCacheFileWatch::slotUpdateDirWatch()
{
    // Event comes from main thread, we need to lock ourselves.
    LoadingCache::CacheLock lock(m_cache);

    // get a list of files in cache that need watch
    QStringList toBeAdded;
    QStringList toBeRemoved = watchedFiles;

    QList<QString> filePaths = m_cache->filePathsInCache();
    foreach(const QString &watchPath, filePaths)
    {
        if (!watchPath.isEmpty())
        {
            if (!watchedFiles.contains(watchPath))
                toBeAdded.append(watchPath);
            toBeRemoved.removeAll(watchPath);
        }
    }

    for (QStringList::iterator it = toBeRemoved.begin(); it != toBeRemoved.end(); ++it)
    {
        //kDebug(50003) << "removing watch for " << *it << endl;
        watch->removeFile(*it);
        watchedFiles.removeAll(*it);
    }

    for (QStringList::iterator it = toBeAdded.begin(); it != toBeAdded.end(); ++it)
    {
        //kDebug(50003) << "adding watch for " << *it << endl;
        watch->addFile(*it);
        watchedFiles.append(*it);
    }

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
