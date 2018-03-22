/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2005-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

// Local includes

#include "digikam_debug.h"
#include "iccmanager.h"
#include "icctransform.h"
#include "loadsavethread.h"
#include "managedloadsavethread.h"
#include "sharedloadsavethread.h"
#include "loadingcache.h"

namespace Digikam
{

void LoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        return;
    }

    DImg img(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
    m_thread->taskHasFinished();
    m_thread->imageLoaded(m_loadingDescription, img);
}

LoadingTask::TaskType LoadingTask::type()
{
    return TaskTypeLoading;
}

void LoadingTask::progressInfo(const DImg* const, float progress)
{
    if (m_loadingTaskStatus == LoadingTaskStatusLoading)
    {
        if (m_thread->querySendNotifyEvent())
        {
            m_thread->loadingProgress(m_loadingDescription, progress);
        }
    }
}

bool LoadingTask::continueQuery(const DImg* const)
{
    return m_loadingTaskStatus != LoadingTaskStatusStopping;
}

void LoadingTask::setStatus(LoadingTaskStatus status)
{
    m_loadingTaskStatus = status;
}

//---------------------------------------------------------------------------------------------------

SharedLoadingTask::SharedLoadingTask(LoadSaveThread* const thread, const LoadingDescription& description,
                                     LoadSaveThread::AccessMode mode, LoadingTaskStatus loadingTaskStatus)
    : LoadingTask(thread, description, loadingTaskStatus),
      m_completed(false),
      m_accessMode(mode),
      m_usedProcess(0),
      m_resultLoadingDescription(description)
{
    if (m_accessMode == LoadSaveThread::AccessModeRead && needsPostProcessing())
    {
        m_accessMode = LoadSaveThread::AccessModeReadWrite;
    }
}

void SharedLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
    {
        return;
    }

    // send StartedLoadingEvent from each single Task, not via LoadingProcess list
    m_thread->imageStartedLoading(m_loadingDescription);

    LoadingCache* cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images
        DImg* cachedImg        = 0;
        QStringList lookupKeys = m_loadingDescription.lookupCacheKeys();

        foreach(const QString& key, lookupKeys)
        {
            if ((cachedImg = cache->retrieveImage(key)))
            {
                if (m_loadingDescription.needCheckRawDecoding())
                {
                    if (cachedImg->rawDecodingSettings() == m_loadingDescription.rawDecodingSettings)
                    {
                        break;
                    }
                    else
                    {
                        cachedImg = 0;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if (cachedImg)
        {
            // image is found in image cache, loading is successful
            m_img = *cachedImg;

            if (accessMode() == LoadSaveThread::AccessModeReadWrite)
            {
                m_img = m_img.copy();
            }

            // continues after else clause...
        }
        else
        {
            // find possible running loading process
            m_usedProcess = 0;

            for (QStringList::const_iterator it = lookupKeys.constBegin() ; it != lookupKeys.constEnd() ; ++it)
            {
                if ((m_usedProcess = cache->retrieveLoadingProcess(*it)))
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
                while (m_loadingTaskStatus != LoadingTaskStatusStopping && m_usedProcess && !m_usedProcess->completed())
                {
                    lock.timedWait();
                }

                // remove listener from process
                if (m_usedProcess)
                {
                    m_usedProcess->removeListener(this);
                }

                // wake up the process which is waiting until all listeners have removed themselves
                lock.wakeAll();
                // set to 0, as checked in setStatus
                m_usedProcess = 0;
                //qCDebug(DIGIKAM_GENERAL_LOG) << "SharedLoadingTask " << this << ": waited";
                // m_img is now set to the result
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

    if (!m_img.isNull())
    {
        // following the golden rule to avoid deadlocks, do this when CacheLock is not held
        postProcess();
        m_thread->taskHasFinished();
        m_thread->imageLoaded(m_resultLoadingDescription, m_img);
        return;
    }

    // load image
    m_img = DImg(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);

//    bool isCached = false;
    {
        LoadingCache::CacheLock lock(cache);

        // put (valid) image into cache of loaded images
        if (!m_img.isNull())
        {
//            isCached =
            cache->putImage(m_loadingDescription.cacheKey(), new DImg(m_img), m_loadingDescription.filePath);
        }

        // remove this from the list of loading processes in cache
        cache->removeLoadingProcess(this);
    }

    {
        LoadingCache::CacheLock lock(cache);
        //qCDebug(DIGIKAM_GENERAL_LOG) << "SharedLoadingTask " << this << ": image loaded, " << img.isNull();
        // indicate that loading has finished so that listeners can stop waiting
        m_completed = true;

        // dispatch image to all listeners, including this
        for (int i = 0 ; i < m_listeners.count() ; ++i)
        {
            LoadingProcessListener* l = m_listeners[i];

            if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
            {
                // If a listener requested ReadWrite access, it gets a deep copy.
                // DImg is explicitly shared.
                DImg copy = m_img.copy();
                l->setResult(m_loadingDescription, copy);
            }
            else
            {
                l->setResult(m_loadingDescription, m_img);
            }
        }

        // remove myself from list of listeners
        removeListener(this);
        // wake all listeners waiting on cache condVar, so that they remove themselves
        lock.wakeAll();

        // wait until all listeners have removed themselves
        while (m_listeners.count() != 0)
        {
            lock.timedWait();
        }

        // set to 0, as checked in setStatus
        m_usedProcess = 0;
    }

    // again: following the golden rule to avoid deadlocks, do this when CacheLock is not held
    postProcess();
    m_thread->taskHasFinished();
    m_thread->imageLoaded(m_loadingDescription, m_img);
}

void SharedLoadingTask::setResult(const LoadingDescription& loadingDescription, const DImg& img)
{
    // this is called from another process's execute while this task is waiting on m_usedProcess.
    // Note that loadingDescription need not equal m_loadingDescription (may be superior)
    m_resultLoadingDescription                          = loadingDescription;
    // these are taken from our own description
    m_resultLoadingDescription.postProcessingParameters = m_loadingDescription.postProcessingParameters;
    m_img                                               = img;
}

bool SharedLoadingTask::needsPostProcessing() const
{
    return m_loadingDescription.postProcessingParameters.needsProcessing();
}

void SharedLoadingTask::postProcess()
{
    // to receive progress info again. Should be safe now, we are alone.
    addListener(this);

    // ---- Color management ---- //

    switch (m_loadingDescription.postProcessingParameters.colorManagement)
    {
        case LoadingDescription::NoColorConversion:
            break;
        case LoadingDescription::ApplyTransform:
        {
            IccTransform trans = m_loadingDescription.postProcessingParameters.transform();
            trans.apply(m_img, this);
            m_img.setIccProfile(trans.outputProfile());
            break;
        }
        case LoadingDescription::ConvertForEditor:
        {
            IccManager manager(m_img);
            manager.transformDefault();
            break;
        }
        case LoadingDescription::ConvertToSRGB:
        {
            IccManager manager(m_img);
            manager.transformToSRGB();
            break;
        }
        case LoadingDescription::ConvertForDisplay:
        {
            IccManager manager(m_img);
            manager.transformForDisplay(m_loadingDescription.postProcessingParameters.profile());
            break;
        }
        case LoadingDescription::ConvertForOutput:
        {
            IccManager manager(m_img);
            manager.transformForOutput(m_loadingDescription.postProcessingParameters.profile());
            break;
        }
    }

    removeListener(this);
}

void SharedLoadingTask::progressInfo(const DImg* const, float progress)
{
    if (m_loadingTaskStatus == LoadingTaskStatusLoading)
    {
        LoadingCache* cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        for (int i = 0 ; i < m_listeners.size() ; ++i)
        {
            LoadingProcessListener* l  = m_listeners[i];
            LoadSaveNotifier* notifier = l->loadSaveNotifier();

            if (notifier && l->querySendNotifyEvent())
            {
                notifier->loadingProgress(m_loadingDescription, progress);
            }
        }
    }
}

bool SharedLoadingTask::continueQuery(const DImg* const)
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
        LoadingCache* cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        // check for m_usedProcess, to avoid race condition that it has finished before
        if (m_usedProcess)
        {
            // remove this from list of listeners - check in continueQuery() of active thread
            m_usedProcess->removeListener(this);
            // set m_usedProcess to 0, signalling that we have detached already
            m_usedProcess = 0;
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

void SharedLoadingTask::addListener(LoadingProcessListener* listener)
{
    m_listeners << listener;
}

void SharedLoadingTask::removeListener(LoadingProcessListener* listener)
{
    m_listeners.removeAll(listener);
}

void SharedLoadingTask::notifyNewLoadingProcess(LoadingProcess* process, const LoadingDescription& description)
{
    // Ok, we are notified that another task has been started in another thread.
    // We are of course only interested if the task loads the same file,
    // and we are right now loading a reduced version, and the other task is loading the full version.
    // In this case, we notify our own thread (a signal to the API user is emitted) of this.
    // The fact that we are receiving the method call shows that this task is registered with the LoadingCache,
    // somewhere in between the calls to addLoadingProcess(this) and removeLoadingProcess(this) above.
    if (process != this                                              &&
        m_loadingDescription.isReducedVersion()                      &&
        m_loadingDescription.equalsIgnoreReducedVersion(description) &&
        !description.isReducedVersion())
    {
        for (int i = 0 ; i < m_listeners.size() ; ++i)
        {
            m_listeners[i]->loadSaveNotifier()->moreCompleteLoadingAvailable(m_loadingDescription, description);
        }
    }
}

bool SharedLoadingTask::querySendNotifyEvent()
{
    return m_thread && m_thread->querySendNotifyEvent();
}

LoadSaveNotifier* SharedLoadingTask::loadSaveNotifier()
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
    m_thread->imageStartedSaving(m_filePath);
    bool success = m_img.save(m_filePath, m_format, this);
    m_thread->taskHasFinished();
    m_thread->imageSaved(m_filePath, success);
}

LoadingTask::TaskType SavingTask::type()
{
    return TaskTypeSaving;
}

void SavingTask::progressInfo(const DImg* const, float progress)
{
    if (m_thread->querySendNotifyEvent())
    {
        m_thread->savingProgress(m_filePath, progress);
    }
}

bool SavingTask::continueQuery(const DImg* const)
{
    return (m_savingTaskStatus != SavingTaskStatusStopping);
}

void SavingTask::setStatus(SavingTaskStatus status)
{
    m_savingTaskStatus = status;
}

}   //namespace Digikam
