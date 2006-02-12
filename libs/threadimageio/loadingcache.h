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

#ifndef LOADING_CACHE_H
#define LOADING_CACHE_H

#include <qptrlist.h>
#include <qcache.h>
#include <qdict.h>
#include <qmutex.h>

#include "dimg.h"
#include "loadsavethread.h"


namespace Digikam
{

class LoadingProcessListener
{
public:

    virtual bool querySendNotifyEvent() = 0;
    virtual QObject *eventReceiver() = 0;
    virtual LoadSaveThread::AccessMode accessMode() = 0;

};

class LoadingProcess
{
public:

    virtual bool completed() = 0;
    virtual const QString &filePath() = 0;
    virtual void addListener(LoadingProcessListener *listener) = 0;
    virtual void removeListener(LoadingProcessListener *listener) = 0;

};

class LoadingCachePriv;

class LoadingCache
{
public:

    static LoadingCache *cache();
    static void cleanUp();
    ~LoadingCache();

    // all functions shall only be called when a CacheLock is held
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

    // Retrieves an image for the given string from the cache,
    // or 0 if no image is found.
    DImg *retrieveImage(const QString &filePath);
    // Returns whether the given DImg fits in the cache.
    bool isCacheable(const DImg *img);
    // Put image into for given string into the cache.
    // Returns true if image has been put in the cache, false otherwise.
    // Ownership of the DImg instance is passed to the cache.
    // When it cannot be put in the cache it is deleted.
    bool putImage(const QString &filePath, DImg *img);
    void removeImage(const QString &filePath);
    void removeImages();

    LoadingProcess *retrieveLoadingProcess(const QString &filePath);
    void addLoadingProcess(LoadingProcess *process);
    void removeLoadingProcess(LoadingProcess *process);

    void setCacheSize(int megabytes);

private:

    static LoadingCache *m_instance;

    LoadingCache();

    friend class CacheLock;
    LoadingCachePriv *d;

};

}   // namespace Digikam

#endif
