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

#ifndef LOADING_CACHE_H
#define LOADING_CACHE_H

// Qt includes.

#include <QPixmap>

// Digikam includes.

#include "dimg.h"
#include "loadsavethread.h"

namespace Digikam
{

class LoadingProcessListener
{
public:

    virtual ~LoadingProcessListener() {}
    virtual bool querySendNotifyEvent() = 0;
    virtual LoadSaveNotifier *loadSaveNotifier() = 0;
    virtual LoadSaveThread::AccessMode accessMode() = 0;

};

class LoadingProcess
{
public:

    virtual ~LoadingProcess() {};
    virtual bool completed() = 0;
    virtual QString filePath() = 0;
    virtual QString cacheKey() = 0;
    virtual void addListener(LoadingProcessListener *listener) = 0;
    virtual void removeListener(LoadingProcessListener *listener) = 0;
    virtual void notifyNewLoadingProcess(LoadingProcess *process, LoadingDescription description) = 0;

};

class LoadingCachePriv;

class LoadingCache : public QObject
{

    Q_OBJECT

public:

    static LoadingCache *cache();
    static void cleanUp();
    virtual ~LoadingCache();

    /// !! All methods of LoadingCache shall only be called when a CacheLock is held !!

    class CacheLock
    {
    public:
        CacheLock(LoadingCache *cache);
        ~CacheLock();
        void wakeAll();
        void timedWait();
    private:
        LoadingCache *m_cache;
    };

    /**
     * Retrieves an image for the given string from the cache,
     * or 0 if no image is found.
     */
    DImg *retrieveImage(const QString &cacheKey);
    /// Returns whether the given DImg fits in the cache.
    bool isCacheable(const DImg *img);
    /** Put image into for given string into the cache.
     *  Returns true if image has been put in the cache, false otherwise.
     *  Ownership of the DImg instance is passed to the cache.
     *  When it cannot be put in the cache it is deleted.
     *  The third parameter specifies a file path that will be watched.
     *  If this file changes, the object will be removed from the cache.
     */
    bool putImage(const QString &cacheKey, DImg *img, const QString &filePath);
    /**
     *  Remove entries for the given cacheKey from the cache
     */
    void removeImage(const QString &cacheKey);
    /**
     *  Remove all entries from the cache
     */
    void removeImages();

    // ------- Loading process management -----------------------------------

    /**
     *  Find the loading process for given cacheKey, or 0 if not found
     */
    LoadingProcess *retrieveLoadingProcess(const QString &cacheKey);
    /**
     *  Add a loading process to the list. Only one loading process
     *  for the same cache key is registered at a time.
     */
    void addLoadingProcess(LoadingProcess *process);
    /**
     *  Remove loading process for given cache key
     */
    void removeLoadingProcess(LoadingProcess *process);
    /**
     *  Notify all currently registered loading processes
     */
    void notifyNewLoadingProcess(LoadingProcess *process, LoadingDescription description);

    /**
     *  Sets the cache size in megabytes.
     *  The thumbnail cache is not affected and setThumbnailCacheSize takes the maximum number.
     */
    void setCacheSize(int megabytes);

    // ------- Thumbnail cache -----------------------------------

    /// The LoadingCache support both the caching of QImage and QPixmap objects.
    /// QPixmaps can only be accessed from the main thread, so the tasks cannot access this cache.
    /**
     * Retrieves a thumbnail for the given filePath from the thumbnail cache,
     * or a 0 if the thumbnail is not found.
     */
    const QImage *retrieveThumbnail(const QString &cacheKey);
    const QPixmap *retrieveThumbnailPixmap(const QString &cacheKey);
    /**
     * Puts a thumbnail into the thumbnail cache.
     */
    void putThumbnail(const QString &cacheKey, const QImage  &thumb);
    void putThumbnail(const QString &cacheKey, const QPixmap &thumb);
    /**
     * Remove the thumbnail for the given file path from the thumbnail cache
     */
    void removeThumbnail(const QString &cacheKey);
    /**
     * Remove all thumbnails
     */
    void removeThumbnails();
    /**
     * Sets the size of the thumbnail cache
     *  @param numberOfImages The maximum number of thumbnails of size 256 in QImage format
                              that will be cached. If the size of the images is smaller, a larger
                              number will be cached.
     *  @param numberOfPixmaps The maximum number of thumbnails of size 256 in QPixmap format
                              that will be cached. If the size of the images is smaller, a larger
                              number will be cached.
     * Note: The main cache is unaffected by this method,
     *       and setCacheSize takes megabytes as parameter.
     * Note: A good caching strategy will be to set one of the numbers to 0
     * Default values: (0, 100)
     */
    void setThumbnailCacheSize(int numberOfQImages, int numberOfQPixmaps);

private slots:

    void slotFileDirty(const QString &path);
    void slotUpdateDirWatch();

signals:

    void signalUpdateDirWatch();

private:

    static LoadingCache *m_instance;

    LoadingCache();

    friend class CacheLock;
    LoadingCachePriv *d;

};

}   // namespace Digikam

#endif // LOADING_CACHE_H
