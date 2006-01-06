/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * Copyright 2005 by Marcel Wiesweg, Gilles Caulier
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

    LoadingTask(LoadSaveThread* thread, const QString &filePath, LoadingTaskStatus loadingTaskStatus = LoadingTaskStatusLoading)
        : Task(thread), m_filePath(filePath), m_loadingTaskStatus(loadingTaskStatus)
        {}

    // Task

    virtual void execute()
    {
        if (m_loadingTaskStatus == LoadingTaskStatusStopping)
            return;
        DImg img(m_filePath, this);
        QApplication::postEvent(m_thread, new LoadedEvent(m_filePath, img));
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
                QApplication::postEvent(m_thread, new LoadingProgressEvent(m_filePath, progress));
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
        return m_filePath;
    }

    void setStatus(LoadingTaskStatus status)
    {
        m_loadingTaskStatus = status;
    }
private:

    QString m_filePath;
    LoadingTaskStatus m_loadingTaskStatus;
};

//---------------------------------------------------------------------------------------------------

class SavedEvent : public NotifyEvent
{
public:
    
    SavedEvent(const QString &filePath)
        : m_filePath(filePath)
        {};
    
    virtual void notify(LoadSaveThread *thread)
    {
        thread->imageSaved(m_filePath);
    };
    
private:

    QString m_filePath;
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
        m_img.save(m_filePath, m_format);
        QApplication::postEvent(m_thread, new SavedEvent(m_filePath));
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

LoadSaveThread::LoadSaveThread()
{
    m_currentTask        = 0;
    m_running            = true;

    m_notificationPolicy = NotificationPolicyTimeLimited;
    m_blockNotification  = false;

    start();
}

LoadSaveThread::~LoadSaveThread()
{
    m_running = false;
    {
        QMutexLocker lock(&m_mutex);
        m_condVar.wakeAll();
    }

    wait();
}

void LoadSaveThread::load(const QString& filePath)
{
    QMutexLocker lock(&m_mutex);
    m_todo.append(new LoadingTask(this, filePath));
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
    while (m_running)
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
                    m_notificationTime = QTime::currentTime();
                    m_blockNotification = true;
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
                m_blockNotification = false;
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
    m_blockNotification  = false;
}

bool LoadSaveThread::querySendNotifyEvent()
{
    // This function is called from the thread to ask for permission to send a notify event.
    switch (m_notificationPolicy)
    {
        case NotificationPolicyDirect:
            // Note that m_blockNotification is not protected by a mutex. However, if there is a
            // race condition, the worst case is that one event is not sent, which is no problem.
            if (m_blockNotification)
                return false;
            else
            {
                m_blockNotification = true;
                return true;
            }
            break;
        case NotificationPolicyTimeLimited:
            // Current default time value: 100 millisecs.
            if (m_blockNotification)
                m_blockNotification = m_notificationTime.msecsTo(QTime::currentTime()) < 100;

            if (m_blockNotification)
                return false;
            else
            {
                m_notificationTime = QTime::currentTime();
                m_blockNotification = true;
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
            removeLoadingTasks(QString(), LoadingTaskFilterAll);
            break;
        }
        case TerminationPolicyTerminatePreloading:
        {
            QMutexLocker lock(&m_mutex);
            if ( (loadingTask = checkLoadingTask(m_currentTask, LoadingTaskFilterPreloading)) )
                loadingTask->setStatus(LoadingTask::LoadingTaskStatusStopping);
            removeLoadingTasks(QString(), LoadingTaskFilterPreloading);
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

LoadingTask *ManagedLoadSaveThread::findExistingTask(const QString& filePath)
{
    LoadingTask *loadingTask;
    if (m_currentTask)
    {
        if (m_currentTask->type() == Task::TaskTypeLoading)
        {
            loadingTask = (LoadingTask *)m_currentTask;
            if (loadingTask->filePath() == filePath)
                return loadingTask;
        }
    }
    for (Task *task = m_todo.first(); task; task = m_todo.next())
    {
        if (task->type() == Task::TaskTypeLoading)
        {
            loadingTask = (LoadingTask *)task;
            if (loadingTask->filePath() == filePath)
                return loadingTask;
        }
    }
    return 0;
}

void ManagedLoadSaveThread::setTerminationPolicy(TerminationPolicy terminationPolicy)
{
    m_terminationPolicy = terminationPolicy;
}

void ManagedLoadSaveThread::load(const QString& filePath, LoadingPolicy policy)
{
    QMutexLocker lock(&m_mutex);
    LoadingTask *loadingTask = 0;
    LoadingTask *existingTask = findExistingTask(filePath);

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
            m_todo.append(new LoadingTask(this, filePath));
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
            m_todo.prepend(new LoadingTask(this, filePath));
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
                    m_todo.insert(i, new LoadingTask(this, filePath));
                    break;
                }
            }
            break;
        case LoadingPolicyPreload:
            // append to the very end of the list
            //kdDebug() << "LoadingPolicyPreload, Existing task " << existingTask << endl;
            if (existingTask)
                break;
            m_todo.append(new LoadingTask(this, filePath, LoadingTask::LoadingTaskStatusPreloading));
            break;
    }
    m_condVar.wakeAll();
}

void ManagedLoadSaveThread::stopLoading(const QString& filePath, LoadingTaskFilter filter)
{
    QMutexLocker lock(&m_mutex);
    removeLoadingTasks(filePath, filter);
}

void ManagedLoadSaveThread::removeLoadingTasks(const QString& filePath, LoadingTaskFilter filter)
{
    LoadingTask *loadingTask;

    // stop current task if it is matching the criteria
    if ( (loadingTask = checkLoadingTask(m_currentTask, filter)) )
    {
        if (filePath.isNull() || loadingTask->filePath() == filePath)
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
            if (filePath.isNull() || loadingTask->filePath() == filePath)
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



}   // namespace Digikam

#include "loadsavethread.moc"
