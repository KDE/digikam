/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * Copyright 2005-2006 by Marcel Wiesweg, Gilles Caulier
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

#include "loadsavetask.h"

// Qt includes.

#include <qapplication.h>

// Locale includes.

#include "ddebug.h"
#include "loadsavethread.h"
#include "managedloadsavethread.h"
#include "sharedloadsavethread.h"
#include "loadingcache.h"

namespace Digikam
{

void LoadingProgressEvent::notify(LoadSaveThread *thread)
{
    thread->loadingProgress(m_loadingDescription, m_progress);
}

void SavingProgressEvent::notify(LoadSaveThread *thread)
{
    thread->savingProgress(m_filePath, m_progress);
}

void StartedLoadingEvent::notify(LoadSaveThread *thread)
{
    thread->imageStartedLoading(m_loadingDescription);
}

void StartedSavingEvent::notify(LoadSaveThread *thread)
{
    thread->imageStartedSaving(m_filePath);
}

void LoadedEvent::notify(LoadSaveThread *thread)
{
    thread->imageLoaded(m_loadingDescription, m_img);
}

void MoreCompleteLoadingAvailableEvent::notify(LoadSaveThread *thread)
{
    thread->moreCompleteLoadingAvailable(m_oldDescription, m_newDescription);
}

void SavedEvent::notify(LoadSaveThread *thread)
{
    thread->imageSaved(m_filePath, m_success);
}

//---------------------------------------------------------------------------------------------------

void LoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
        return;
    DImg img(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
    m_thread->taskHasFinished();
    QApplication::postEvent(m_thread, new LoadedEvent(m_loadingDescription.filePath, img));
}

LoadingTask::TaskType LoadingTask::type()
{
    return TaskTypeLoading;
}

void LoadingTask::progressInfo(const DImg *, float progress)
{
    if (m_loadingTaskStatus == LoadingTaskStatusLoading)
    {
        if (m_thread->querySendNotifyEvent())
            QApplication::postEvent(m_thread, new LoadingProgressEvent(m_loadingDescription.filePath, progress));
    }
}

bool LoadingTask::continueQuery(const DImg *)
{
    return m_loadingTaskStatus != LoadingTaskStatusStopping;
}

void LoadingTask::setStatus(LoadingTaskStatus status)
{
    m_loadingTaskStatus = status;
}


// This is a hack needed to prevent hanging when a KProcess-based loader (raw loader)
// is waiting for the process to finish, but the main thread is waiting
// for the thread to finish and no KProcess events are delivered.
// Remove when porting to Qt4.
bool LoadingTask::isShuttingDown()
{
    return m_thread->isShuttingDown();
}

//---------------------------------------------------------------------------------------------------

void SharedLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
        return;
    // send StartedLoadingEvent from each single Task, not via LoadingProcess list
    QApplication::postEvent(m_thread, new StartedLoadingEvent(m_loadingDescription.filePath));

    LoadingCache *cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images
        DImg *cachedImg = 0;
        QStringList lookupKeys = m_loadingDescription.lookupCacheKeys();
        for ( QStringList::Iterator it = lookupKeys.begin(); it != lookupKeys.end(); ++it ) {
            if ( (cachedImg = cache->retrieveImage(*it)) )
                break;
        }

        if (cachedImg)
        {
            // image is found in image cache, loading is successfull
            DImg img(*cachedImg);
            if (accessMode() == LoadSaveThread::AccessModeReadWrite)
                img = img.copy();
            QApplication::postEvent(m_thread, new LoadedEvent(m_loadingDescription.filePath, img));
            return;
        }
        else
        {
            // find possible running loading process
            m_usedProcess = 0;
            for ( QStringList::Iterator it = lookupKeys.begin(); it != lookupKeys.end(); ++it ) {
                if ( (m_usedProcess = cache->retrieveLoadingProcess(*it)) )
                {
                    break;
                }
            }

            if (m_usedProcess)
            {
                // Other process is right now loading this image.
                // Add this task to the list of listeners and
                // attach this thread to the other thread, wait until loading
                // has finished.
                m_usedProcess->addListener(this);
                // break loop when either the loading has completed, or this task is being stopped
                while ( !m_usedProcess->completed() && m_loadingTaskStatus != LoadingTaskStatusStopping )
                    lock.timedWait();
                // remove listener from process
                m_usedProcess->removeListener(this);
                // wake up the process which is waiting until all listeners have removed themselves
                lock.wakeAll();
                // set to 0, as checked in setStatus
                m_usedProcess = 0;
                //DDebug() << "SharedLoadingTask " << this << ": waited" << endl;
                return;
            }
            else
            {
                // Neither in cache, nor currently loading in different thread.
                // Load it here and now, add this LoadingProcess to cache list.
                cache->addLoadingProcess(this);
                // Add this to the list of listeners
                addListener(this);
                // for use in setStatus
                m_usedProcess = this;
                // Notify other processes that we are now loading this image.
                // They might be interested - see notifyNewLoadingProcess below
                cache->notifyNewLoadingProcess(this, m_loadingDescription);
            }
        }
    }

    // load image
    DImg img(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);

    bool isCached = false;
    {
        LoadingCache::CacheLock lock(cache);
        // put (valid) image into cache of loaded images
        if (!img.isNull())
            isCached = cache->putImage(m_loadingDescription.cacheKey(), new DImg(img), m_loadingDescription.filePath);
        // remove this from the list of loading processes in cache
        cache->removeLoadingProcess(this);
    }

    // following the golden rule to avoid deadlocks, do this when CacheLock is not held
    m_thread->taskHasFinished();

    {
        LoadingCache::CacheLock lock(cache);
        //DDebug() << "SharedLoadingTask " << this << ": image loaded, " << img.isNull() << endl;
        // indicate that loading has finished so that listeners can stop waiting
        m_completed = true;

        // Optimize so that no unnecessary copying is done.
        // If image has been put in cache, the initial copy has been consumed for this.
        // If image is too large for cache, the initial copy is still available.
        bool usedInitialCopy = isCached;
        // dispatch image to all listeners, including this
        for (LoadingProcessListener *l = m_listeners.first(); l; l = m_listeners.next())
        {
            // This code sends a copy only when ReadWrite access is requested.
            // Otherwise, the image from the cache is sent.
            // As the image in the cache will be deleted from any thread, the explicit sharing
            // needs to be thread-safe to avoid the risk of memory leaks.
            // This is the case only for Qt4, so uncomment this code when porting.
            /*
            if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
            {
                // If a listener requested ReadWrite access, it gets a deep copy.
                // DImg is explicitly shared.
                DImg copy = img.copy();
                QApplication::postEvent(l->eventReceiver(), new LoadedEvent(m_loadingDescription.filePath, copy));
            }
            else
                QApplication::postEvent(l->eventReceiver(), new LoadedEvent(m_loadingDescription.filePath, img));
            */
            // Qt3: The same copy for all Read listeners (it is assumed that they will delete it only in the main thread),
            // an extra copy for each ReadWrite listener
            DImg readerCopy;
            if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
            {
                // If a listener requested ReadWrite access, it gets a deep copy.
                // DImg is explicitly shared.
                DImg copy;
                if (usedInitialCopy)
                {
                    copy = img.copy();
                }
                else
                {
                    copy = img;
                    usedInitialCopy = true;
                }
                QApplication::postEvent(l->eventReceiver(), new LoadedEvent(m_loadingDescription, copy));
            }
            else
            {
                if (readerCopy.isNull())
                {
                    if (usedInitialCopy)
                    {
                        readerCopy = img.copy();
                    }
                    else
                    {
                        readerCopy = img;
                        usedInitialCopy = true;
                    }
                }
                QApplication::postEvent(l->eventReceiver(), new LoadedEvent(m_loadingDescription, readerCopy));
            }
        }

        // remove myself from list of listeners
        removeListener(this);
        // wake all listeners waiting on cache condVar, so that they remove themselves
        lock.wakeAll();
        // wait until all listeners have removed themselves
        while (m_listeners.count() != 0)
            lock.timedWait();
        // set to 0, as checked in setStatus
        m_usedProcess = 0;
    }
}

void SharedLoadingTask::progressInfo(const DImg *, float progress)
{
    if (m_loadingTaskStatus == LoadingTaskStatusLoading)
    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        for (LoadingProcessListener *l = m_listeners.first(); l; l = m_listeners.next())
        {
            if (l->querySendNotifyEvent())
                QApplication::postEvent(l->eventReceiver(), new LoadingProgressEvent(m_loadingDescription, progress));
        }
    }
}

bool SharedLoadingTask::continueQuery(const DImg *)
{
    // If this is called, the thread is currently loading an image.
    // In shared loading, we cannot stop until all listeners have been removed as well
    return (m_loadingTaskStatus != LoadingTaskStatusStopping) || (m_listeners.count() != 0);
}

void SharedLoadingTask::setStatus(LoadingTaskStatus status)
{
    m_loadingTaskStatus = status;
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        // check for m_usedProcess, to avoid race condition that it has finished before
        if (m_usedProcess)
        {
            // remove this from list of listeners - check in continueQuery() of active thread
            m_usedProcess->removeListener(this);
            // wake all listeners - particularly this - from waiting on cache condvar
            lock.wakeAll();
        }
    }
}

bool SharedLoadingTask::completed()
{
    return m_completed;
}

QString SharedLoadingTask::filePath()
{
    return m_loadingDescription.filePath;
}

QString SharedLoadingTask::cacheKey()
{
    return m_loadingDescription.cacheKey();
}

void SharedLoadingTask::addListener(LoadingProcessListener *listener)
{
    m_listeners.append(listener);
}

void SharedLoadingTask::removeListener(LoadingProcessListener *listener)
{
    m_listeners.remove(listener);
}

void SharedLoadingTask::notifyNewLoadingProcess(LoadingProcess *process, LoadingDescription description)
{
    // Ok, we are notified that another task has been started in another thread.
    // We are of course only interested if the task loads the same file,
    // and we are right now loading a reduced version, and the other task is loading the full version.
    // In this case, we notify our own thread (a signal to the API user is emitted) of this.
    // The fact that we are receiving the method call shows that this task is registered with the LoadingCache,
    // somewhere in between the calls to addLoadingProcess(this) and removeLoadingProcess(this) above.
    if (process != this &&
        m_loadingDescription.isReducedVersion() &&
        m_loadingDescription.equalsIgnoreReducedVersion(description) &&
        !description.isReducedVersion()
       )
    {
        for (LoadingProcessListener *l = m_listeners.first(); l; l = m_listeners.next())
        {
            QApplication::postEvent(l->eventReceiver(), new MoreCompleteLoadingAvailableEvent(m_loadingDescription, description));
        }
    }
}

bool SharedLoadingTask::querySendNotifyEvent()
{
    return m_thread->querySendNotifyEvent();
}

QObject *SharedLoadingTask::eventReceiver()
{
    return m_thread;
}

LoadSaveThread::AccessMode SharedLoadingTask::accessMode()
{
    return m_accessMode;
}

//---------------------------------------------------------------------------------------------------

void SavingTask::execute()
{
    bool success = m_img.save(m_filePath, m_format, this);
    m_thread->taskHasFinished();
    QApplication::postEvent(m_thread, new SavedEvent(m_filePath, success));
};

LoadingTask::TaskType SavingTask::type()
{
    return TaskTypeSaving;
}

void SavingTask::progressInfo(const DImg *, float progress)
{
    if (m_thread->querySendNotifyEvent())
        QApplication::postEvent(m_thread, new SavingProgressEvent(m_filePath, progress));
}

bool SavingTask::continueQuery(const DImg *)
{
    return m_savingTaskStatus != SavingTaskStatusStopping;
}

void SavingTask::setStatus(SavingTaskStatus status)
{
    m_savingTaskStatus = status;
}

}   //namespace Digikam
