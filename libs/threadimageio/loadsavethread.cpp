/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at free.fr>
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

// Qt includes.

#include <qapplication.h>

// KDE includes

#include <kdebug.h>

// Locale includes.

#include "loadsavethread.h"
#include "managedloadsavethread.h"
#include "sharedloadsavethread.h"
#include "loadingcache.h"

namespace Digikam
{

class Task
{
public:

    Task(LoadSaveThread* thread)
        : m_thread(thread)
        {};
    virtual ~Task() {};

    virtual void execute() = 0;

    enum TaskType
    {
        TaskTypeLoading,
        TaskTypeSaving
    };

    virtual TaskType type() = 0;

protected:

    LoadSaveThread *m_thread;
};

//---------------------------------------------------------------------------------------------------

class NotifyEvent : public QCustomEvent
{
public:

    static QEvent::Type notifyEventId()
        { return QEvent::User; };
    
    NotifyEvent() : QCustomEvent(notifyEventId()) {};
    
    virtual void notify(LoadSaveThread *thread) = 0;
};

//---------------------------------------------------------------------------------------------------

class ProgressEvent : public NotifyEvent
{
public:

    ProgressEvent(const QString& filePath, float progress)
        : m_filePath(filePath), m_progress(progress)
        {};

protected:

    QString m_filePath;
    float m_progress;
};

//---------------------------------------------------------------------------------------------------

class LoadingProgressEvent : public ProgressEvent
{
public:

    LoadingProgressEvent(const QString& filePath, float progress)
        : ProgressEvent(filePath, progress)
        {};

    virtual void notify(LoadSaveThread *thread)
        { thread->loadingProgress(m_filePath, m_progress); };
};

//---------------------------------------------------------------------------------------------------

class SavingProgressEvent : public ProgressEvent
{
public:

    SavingProgressEvent(const QString& filePath, float progress)
        : ProgressEvent(filePath, progress)
        {};

    virtual void notify(LoadSaveThread *thread)
        { thread->savingProgress(m_filePath, m_progress); };
};

//---------------------------------------------------------------------------------------------------

class StartedLoadingEvent : public NotifyEvent
{
public:

    StartedLoadingEvent(const QString& filePath)
        : m_filePath(filePath)
        {};

    virtual void notify(LoadSaveThread *thread)
        { thread->imageStartedLoading(m_filePath); };

private:

    QString m_filePath;
};

//---------------------------------------------------------------------------------------------------

class StartedSavingEvent : public NotifyEvent
{
    public:

        StartedSavingEvent(const QString& filePath)
    : m_filePath(filePath)
        {};

        virtual void notify(LoadSaveThread *thread)
        { thread->imageStartedSaving(m_filePath); };

    private:

        QString m_filePath;
};

//---------------------------------------------------------------------------------------------------

class LoadedEvent : public NotifyEvent
{
public:

    LoadedEvent(const QString &filePath, DImg &img)
        : m_filePath(filePath), m_img(img)
        {};
        
    virtual void notify(LoadSaveThread *thread)
        { thread->imageLoaded(m_filePath, m_img); };
        
private:
    
    QString m_filePath;
    DImg    m_img;
};

//---------------------------------------------------------------------------------------------------

class LoadingTask : public Task, public DImgLoaderObserver
{
public:

    enum LoadingTaskStatus
    {
        LoadingTaskStatusLoading,
        LoadingTaskStatusPreloading,
        LoadingTaskStatusStopping
    };

    LoadingTask(LoadSaveThread* thread, LoadingDescription description,
                LoadingTaskStatus loadingTaskStatus = LoadingTaskStatusLoading)
        : Task(thread), m_loadingDescription(description), m_loadingTaskStatus(loadingTaskStatus)
        {}

    // Task

    virtual void execute()
    {
        if (m_loadingTaskStatus == LoadingTaskStatusStopping)
            return;
        DImg img(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
        QApplication::postEvent(m_thread, new LoadedEvent(m_loadingDescription.filePath, img));
    };

    virtual TaskType type()
    {
        return TaskTypeLoading;
    }

    // DImgLoaderObserver

    virtual void progressInfo(const DImg *, float progress)
    {
        if (m_loadingTaskStatus == LoadingTaskStatusLoading)
        {
            if (m_thread->querySendNotifyEvent())
                QApplication::postEvent(m_thread, new LoadingProgressEvent(m_loadingDescription.filePath, progress));
        }
    }

    virtual bool continueQuery(const DImg *)
    {
        return m_loadingTaskStatus != LoadingTaskStatusStopping;
    }


    LoadingTaskStatus status() const
    {
        return m_loadingTaskStatus;
    }

    QString filePath() const
    {
        return m_loadingDescription.filePath;
    }

    LoadingDescription loadingDescription() const
    {
        return m_loadingDescription;
    }

    virtual void setStatus(LoadingTaskStatus status)
    {
        m_loadingTaskStatus = status;
    }
protected:

    LoadingDescription m_loadingDescription;
    LoadingTaskStatus m_loadingTaskStatus;
};

//---------------------------------------------------------------------------------------------------

class SharedLoadingTask : public LoadingTask, public LoadingProcess, public LoadingProcessListener
{
public:

    SharedLoadingTask(LoadSaveThread* thread, LoadingDescription description,
                      LoadSaveThread::AccessMode mode = LoadSaveThread::AccessModeReadWrite,
                      LoadingTaskStatus loadingTaskStatus = LoadingTaskStatusLoading)
        : LoadingTask(thread, description, loadingTaskStatus),
          m_accessMode(mode), m_completed(false), usedProcess(0)
        {}

    virtual void execute()
    {
        if (m_loadingTaskStatus == LoadingTaskStatusStopping)
            return;
        // send StartedLoadingEvent from each single Task, not via LoadingProcess list
        QApplication::postEvent(m_thread, new StartedLoadingEvent(m_loadingDescription.filePath));

        LoadingCache *cache = LoadingCache::cache();
        {
            LoadingCache::CacheLock lock(cache);
            DImg *cachedImg;
            if ( (cachedImg = cache->retrieveImage(m_loadingDescription.filePath)) )
            {
                // image is found in image cache, loading is successfull
                kdDebug() << "SharedLoadingTask " << this << ": " << m_loadingDescription.filePath << " found in image cache" << endl;
                DImg img(*cachedImg);
                QApplication::postEvent(m_thread, new LoadedEvent(m_loadingDescription.filePath, img));
                return;
            }
            else if ( (usedProcess = cache->retrieveLoadingProcess(m_loadingDescription.filePath)) )
            {
                // Other process is right now loading this image.
                // Add this task to the list of listeners and
                // attach this thread to the other thread, wait until loading
                // has finished.
                kdDebug() << "SharedLoadingTask " << this << ": " << m_loadingDescription.filePath << " currently loading, waiting..." << endl;
                usedProcess->addListener(this);
                // break loop when either the loading has completed, or this task is being stopped
                while ( !usedProcess->completed() && m_loadingTaskStatus != LoadingTaskStatusStopping )
                    lock.timedWait();
                // remove listener from process
                usedProcess->removeListener(this);
                // wake up the process which is waiting until all listeners have removed themselves
                lock.wakeAll();
                kdDebug() << "SharedLoadingTask " << this << ": waited" << endl;
                return;
            }
            else
            {
                // Neither in cache, nor currently loading in different thread.
                // Load it here and now, add this LoadingProcess to cache list.
                kdDebug() << "SharedLoadingTask " << this << ": " << m_loadingDescription.filePath << " neither in cache nor loading, loading it now." << endl;
                cache->addLoadingProcess(this);
                // Add this to the list of listeners
                addListener(this);
                // for use in setStatus
                usedProcess = this;
            }
        }

        // load image
        DImg img(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);

        {
            LoadingCache::CacheLock lock(cache);
            kdDebug() << "SharedLoadingTask " << this << ": image loaded, " << img.isNull() << endl;
            // indicate that loading has finished so that listeners can stop waiting
            m_completed = true;
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
                // Qt3: One copy for all Read listeners (it is assumed that they will delete it only in the main thread),
                // one copy for each ReadWrite listener
                DImg readerCopy;
                if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
                {
                    // If a listener requested ReadWrite access, it gets a deep copy.
                    // DImg is explicitly shared.
                    DImg copy = img.copy();
                    QApplication::postEvent(l->eventReceiver(), new LoadedEvent(m_loadingDescription.filePath, copy));
                }
                else
                {
                    if (readerCopy.isNull())
                        readerCopy = img.copy();
                    QApplication::postEvent(l->eventReceiver(), new LoadedEvent(m_loadingDescription.filePath, readerCopy));
                }
            }
            // remove this from the list of loading processes in cache
            cache->removeLoadingProcess(this);
            // put image into cache of loaded images
            cache->putImage(m_loadingDescription.filePath, new DImg(img));
            // wake all listeners waiting on cache condVar
            lock.wakeAll();
            // wait until all listeners have removed themselves
            while (m_listeners.count() != 0)
                lock.timedWait();
        }
    };

    virtual void progressInfo(const DImg *, float progress)
    {
        if (m_loadingTaskStatus == LoadingTaskStatusLoading)
        {
            LoadingCache *cache = LoadingCache::cache();
            LoadingCache::CacheLock lock(cache);

            for (LoadingProcessListener *l = m_listeners.first(); l; l = m_listeners.next())
            {
                if (l->querySendNotifyEvent())
                    QApplication::postEvent(l->eventReceiver(), new LoadingProgressEvent(m_loadingDescription.filePath, progress));
            }
        }
    }

    virtual bool continueQuery(const DImg *)
    {
        // If this is called, the thread is currently loading an image.
        // In shared loading, we cannot stop until all listeners have been removed as well
        return (m_loadingTaskStatus != LoadingTaskStatusStopping) || (m_listeners.count() != 0);
    }

    virtual void setStatus(LoadingTaskStatus status)
    {
        m_loadingTaskStatus = status;
        if (m_loadingTaskStatus == LoadingTaskStatusStopping)
        {
            LoadingCache *cache = LoadingCache::cache();
            LoadingCache::CacheLock lock(cache);
            // remove this from list of listeners - check in continueQuery() of active thread
            usedProcess->removeListener(this);
            // wake all listeners - particularly this - from waiting on cache condvar
            lock.wakeAll();
        }
    }

    // LoadingProcess

    virtual bool completed()
        { return m_completed; };

    virtual const QString &filePath()
    { return m_loadingDescription.filePath; };

    virtual void addListener(LoadingProcessListener *listener)
        { m_listeners.append(listener); };

    virtual void removeListener(LoadingProcessListener *listener)
        { m_listeners.remove(listener); };

    // LoadingProcessListener

    virtual bool querySendNotifyEvent()
    {
        return m_thread->querySendNotifyEvent();
    }

    virtual QObject *eventReceiver()
    {
        return m_thread;
    }

    virtual LoadSaveThread::AccessMode accessMode()
    {
        return m_accessMode;
    }

private:

    LoadSaveThread::AccessMode m_accessMode;
    bool m_completed;
    LoadingProcess *usedProcess;
    QPtrList<LoadingProcessListener> m_listeners;
};

//---------------------------------------------------------------------------------------------------

class SavedEvent : public NotifyEvent
{
public:

    SavedEvent(const QString &filePath, bool success)
        : m_filePath(filePath), m_success(success)
        {};

    virtual void notify(LoadSaveThread *thread)
    {
        thread->imageSaved(m_filePath, m_success);
    };

private:

    QString m_filePath;
    bool m_success;
};

//---------------------------------------------------------------------------------------------------

class SavingTask : public Task, public DImgLoaderObserver
{
public:

    SavingTask(LoadSaveThread* thread, DImg &img, const QString &filePath, const char* format)
        : Task(thread), m_img(img), m_filePath(filePath), m_format(format)
        {};
    
    virtual void execute()
    {
        bool success = m_img.save(m_filePath, m_format);
        QApplication::postEvent(m_thread, new SavedEvent(m_filePath, success));
    };
    
    virtual TaskType type()
    {
        return TaskTypeSaving;
    }

    virtual void progressInfo(const DImg *, float progress)
    {
        if (m_thread->querySendNotifyEvent())
            QApplication::postEvent(m_thread, new SavingProgressEvent(m_filePath, progress));
    }
private:

    DImg        m_img;
    QString     m_filePath;
    const char* m_format;
};

//---------------------------------------------------------------------------------------------------

bool LoadingDescription::operator==(const LoadingDescription &other)
{
    return filePath == other.filePath &&
            rawDecodingSettings.cameraColorBalance    == other.rawDecodingSettings.cameraColorBalance &&
            rawDecodingSettings.automaticColorBalance == other.rawDecodingSettings.automaticColorBalance &&
            rawDecodingSettings.RGBInterpolate4Colors == other.rawDecodingSettings.RGBInterpolate4Colors &&
            rawDecodingSettings.enableRAWQuality      == other.rawDecodingSettings.enableRAWQuality &&
            rawDecodingSettings.RAWQuality            == other.rawDecodingSettings.RAWQuality;
}

//---------------------------------------------------------------------------------------------------

    class LoadSaveThreadPriv
{
public:

    LoadSaveThreadPriv()
    {
        running           = true;
        blockNotification = false;
    }

    bool  running;
    bool  blockNotification;

    QTime notificationTime;
};

//---------------------------------------------------------------------------------------------------

LoadSaveThread::LoadSaveThread()
{
    d = new LoadSaveThreadPriv;
    m_currentTask        = 0;
    m_notificationPolicy = NotificationPolicyTimeLimited;
    
    start();
}

LoadSaveThread::~LoadSaveThread()
{
    d->running = false;
    {
        QMutexLocker lock(&m_mutex);
        m_condVar.wakeAll();
    }

    wait();
    delete d;
}

void LoadSaveThread::load(LoadingDescription description)
{
    QMutexLocker lock(&m_mutex);
    m_todo.append(new LoadingTask(this, description));
    m_condVar.wakeAll();
}

void LoadSaveThread::save(DImg &image, const QString& filePath, const char* format)
{
    QMutexLocker lock(&m_mutex);
    m_todo.append(new SavingTask(this, image, filePath, format));
    m_condVar.wakeAll();
}

void LoadSaveThread::run()
{
    while (d->running)
    {
        {
            QMutexLocker lock(&m_mutex);
            if (m_currentTask)
                delete m_currentTask;
            m_currentTask = m_todo.getFirst();
            if (m_currentTask)
            {
                m_todo.removeFirst();
                if (m_notificationPolicy == NotificationPolicyTimeLimited)
                {
                    // set timing values so that first event is sent only
                    // after an initial time span.
                    d->notificationTime = QTime::currentTime();
                    d->blockNotification = true;
                }
            }
            else
                m_condVar.wait(&m_mutex, 1000);
        }
        if (m_currentTask)
            m_currentTask->execute();
    }
}

void LoadSaveThread::customEvent(QCustomEvent *event)
{
    if (event->type() == NotifyEvent::notifyEventId())
    {
        switch (m_notificationPolicy)
        {
            case NotificationPolicyDirect:
                d->blockNotification = false;
                break;
            case NotificationPolicyTimeLimited:
                break;
        }
        ((NotifyEvent *)event)->notify(this);
    }
}

void LoadSaveThread::setNotificationPolicy(NotificationPolicy notificationPolicy)
{
    m_notificationPolicy = notificationPolicy;
    d->blockNotification  = false;
}

bool LoadSaveThread::querySendNotifyEvent()
{
    // This function is called from the thread to ask for permission to send a notify event.
    switch (m_notificationPolicy)
    {
        case NotificationPolicyDirect:
            // Note that m_blockNotification is not protected by a mutex. However, if there is a
            // race condition, the worst case is that one event is not sent, which is no problem.
            if (d->blockNotification)
                return false;
            else
            {
                d->blockNotification = true;
                return true;
            }
            break;
        case NotificationPolicyTimeLimited:
            // Current default time value: 100 millisecs.
            if (d->blockNotification)
                d->blockNotification = d->notificationTime.msecsTo(QTime::currentTime()) < 100;

            if (d->blockNotification)
                return false;
            else
            {
                d->notificationTime = QTime::currentTime();
                d->blockNotification = true;
                return true;
            }
            break;
    }
    return false;
}


//---------------------------------------------------------------------------------------------------

ManagedLoadSaveThread::ManagedLoadSaveThread()
{
    m_terminationPolicy  = TerminationPolicyTerminateLoading;
}

ManagedLoadSaveThread::~ManagedLoadSaveThread()
{
    LoadingTask *loadingTask;
    switch (m_terminationPolicy)
    {
        case TerminationPolicyTerminateLoading:
        {
            QMutexLocker lock(&m_mutex);
            if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterAll)) )
                loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
            removeLoadingTasks(LoadingDescription(QString()), LoadingTaskFilterAll);
            break;
        }
        case TerminationPolicyTerminatePreloading:
        {
            QMutexLocker lock(&m_mutex);
            if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)) )
                loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
            removeLoadingTasks(LoadingDescription(QString()), LoadingTaskFilterPreloading);
            break;
        }
        case TerminationPolicyWait:
            break;
    }
}

LoadingTask *ManagedLoadSaveThread::checkLoadingTask(Task *task, LoadingTaskFilter filter)
{
    if (task && task->type() == Task::TaskTypeLoading)
    {
        LoadingTask *loadingTask = (LoadingTask *)task;
        if (filter == LoadingTaskFilterAll)
            return loadingTask;
        else if (filter == LoadingTaskFilterPreloading)
            if (loadingTask->status() == LoadingTask::LoadingTaskStatusPreloading)
                return loadingTask;
    }
    return 0;
}

LoadingTask *ManagedLoadSaveThread::findExistingTask(const LoadingDescription &loadingDescription)
{
    LoadingTask *loadingTask;
    if (m_currentTask)
    {
        if (m_currentTask->type() == Task::TaskTypeLoading)
        {
            loadingTask = (LoadingTask *)m_currentTask;
            LoadingDescription taskDescription = loadingTask->loadingDescription();
            if (taskDescription == loadingDescription)
                return loadingTask;
        }
    }
    for (Task *task = m_todo.first(); task; task = m_todo.next())
    {
        if (task->type() == Task::TaskTypeLoading)
        {
            loadingTask = (LoadingTask *)task;
            if (loadingTask->loadingDescription() == loadingDescription)
                return loadingTask;
        }
    }
    return 0;
}

void ManagedLoadSaveThread::setTerminationPolicy(TerminationPolicy terminationPolicy)
{
    m_terminationPolicy = terminationPolicy;
}

void ManagedLoadSaveThread::load(LoadingDescription description, LoadingPolicy policy)
{
    load(description, LoadingModeNormal, policy);
}

void ManagedLoadSaveThread::load(LoadingDescription description, LoadingMode loadingMode, LoadingPolicy policy, AccessMode accessMode)
{
    QMutexLocker lock(&m_mutex);
    LoadingTask *loadingTask = 0;
    LoadingTask *existingTask = findExistingTask(description);

    //kdDebug() << "ManagedLoadSaveThread::load " << filePath << ", policy " << policy << endl;
    switch(policy)
    {
        case LoadingPolicyFirstRemovePrevious:
            // reuse task if it exists
            if (existingTask)
            {
                existingTask->setStatus(LoadingTask::LoadingTaskStatusLoading);
            }
            // stop current task
            if (m_currentTask && m_currentTask != existingTask)
            {
                if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterAll)) )
                    loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
            }
            //kdDebug() << "LoadingPolicyFirstRemovePrevious, Existing task " << existingTask << ", m_currentTask " << m_currentTask << endl;
            // remove all loading tasks
            for (Task *task = m_todo.first(); task; task = m_todo.next())
            {
                if (task != existingTask && checkLoadingTask(task, LoadingTaskFilterAll))
                {
                    //kdDebug() << "Removing task " << task << " from list" << endl;
                    m_todo.remove();
                    m_todo.prev();
                }
            }
            // append new, exclusive loading task
            if (existingTask)
                break;
            m_todo.append(createLoadingTask(description, true, loadingMode, accessMode));
            break;
        case LoadingPolicyPrepend:
            if (existingTask)
            {
                existingTask->setStatus(LoadingTask::LoadingTaskStatusLoading);
            }
            // stop and postpone current task if it is a preloading task
            if (m_currentTask)
            {
                if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)) )
                {
                    loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
                    load(loadingTask->filePath(), LoadingPolicyPreload);
                }
            }
            //kdDebug() << "LoadingPolicyPrepend, Existing task " << existingTask << ", m_currentTask " << m_currentTask << endl;
            // prepend new loading task
            if (existingTask)
                break;
            m_todo.prepend(createLoadingTask(description, true, loadingMode, accessMode));
            break;
        case LoadingPolicyAppend:
            if (existingTask)
            {
                existingTask->setStatus(LoadingTask::LoadingTaskStatusLoading);
            }
            // stop and postpone current task if it is a preloading task
            if (m_currentTask)
            {
                if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)) )
                {
                    loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
                    load(loadingTask->filePath(), LoadingPolicyPreload);
                }
            }
            if (existingTask)
                break;
            //kdDebug() << "LoadingPolicyAppend, Existing task " << existingTask << ", m_currentTask " << m_currentTask << endl;
            // append new loading task, put it in front of preloading tasks
            for (uint i = 0; i<m_todo.count(); i++)
            {
                Task *task = m_todo.at(i);
                if ( (loadingTask = checkLoadingTask(task, LoadingTaskFilterPreloading)) )
                {
                    m_todo.insert(i, createLoadingTask(description, false, loadingMode, accessMode));
                    break;
                }
            }
            break;
        case LoadingPolicyPreload:
            // append to the very end of the list
            //kdDebug() << "LoadingPolicyPreload, Existing task " << existingTask << endl;
            if (existingTask)
                break;
            m_todo.append(createLoadingTask(description, true, loadingMode, accessMode));
            break;
    }
    m_condVar.wakeAll();
}

LoadingTask *ManagedLoadSaveThread::createLoadingTask(const LoadingDescription &description,
         bool preloading, LoadingMode loadingMode, AccessMode accessMode)
{
    if (loadingMode == LoadingModeShared)
    {
        if (preloading)
            return new SharedLoadingTask(this, description, accessMode, LoadingTask::LoadingTaskStatusPreloading);
        else
            return new SharedLoadingTask(this, description, accessMode);
    }
    else
    {
        if (preloading)
            return new LoadingTask(this, description, LoadingTask::LoadingTaskStatusPreloading);
        else
            return new LoadingTask(this, description);
    }
}

void ManagedLoadSaveThread::stopLoading(const QString& filePath, LoadingTaskFilter filter)
{
    QMutexLocker lock(&m_mutex);
    removeLoadingTasks(LoadingDescription(filePath), filter);
}

void ManagedLoadSaveThread::removeLoadingTasks(const LoadingDescription &description, LoadingTaskFilter filter)
{
    LoadingTask *loadingTask;

    // stop current task if it is matching the criteria
    if ( (loadingTask = checkLoadingTask(m_currentTask, filter)) )
    {
        if (description.filePath.isNull() || loadingTask->loadingDescription() == description)
        {
            loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
            return;
        }
    }

    // remove relevant preloading tasks from list
    for (Task *task = m_todo.first(); task; task = m_todo.next())
    {
        if ( (loadingTask = checkLoadingTask(task, filter)) )
        {
            if (description.filePath.isNull() || loadingTask->loadingDescription() == description)
            {
                m_todo.remove();
                m_todo.prev();
                return;
            }
        }
    }
}

void ManagedLoadSaveThread::save(DImg &image, const QString& filePath, const char* format)
{
    QMutexLocker lock(&m_mutex);
    LoadingTask *loadingTask;

    // stop and postpone current task if it is a preloading task
    if (m_currentTask && (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)))
    {
        loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
        load(loadingTask->filePath(), LoadingPolicyPreload);
    }
    // append new loading task, put it in front of preloading tasks
    uint i;
    for (i = 0; i<m_todo.count(); i++)
    {
        Task *task = m_todo.at(i);
        if ( (loadingTask = checkLoadingTask(task, LoadingTaskFilterPreloading)) )
            break;
    }
    m_todo.insert(i, new SavingTask(this, image, filePath, format));
}

void SharedLoadSaveThread::load(LoadingDescription description, AccessMode mode, LoadingPolicy policy)
{
    ManagedLoadSaveThread::load(description, LoadingModeShared, policy, mode);
}

DImg SharedLoadSaveThread::cacheLookup(const QString& filePath, AccessMode /*accessMode*/)
{
    LoadingCache *cache = LoadingCache::cache();
    LoadingCache::CacheLock lock(cache);
    DImg *cachedImg = cache->retrieveImage(filePath);
    // Qt4: uncomment this code.
    // See comments in SharedLoadingTask::execute for explanation.
    /*
    if (cachedImg)
    {
        if (accessMode == AccessModeReadWrite)
            return cachedImg->copy();
        else
            return *cachedImg;
    }
    else
        return DImg();
    */
    if (cachedImg)
        return cachedImg->copy();
    else
        return DImg();
}

}   // namespace Digikam

#include "loadsavethread.moc"
