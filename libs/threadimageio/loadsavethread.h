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

#ifndef LOAD_SAVE_THREAD_H
#define LOAD_SAVE_THREAD_H

// Qt includes.

#include <qthread.h>
#include <qobject.h>
#include <qmutex.h>
#include <qptrlist.h>
#include <qdatetime.h>
#include <qwaitcondition.h>

// Digikam includes.

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LoadSaveThread : public QObject, public QThread
{

    Q_OBJECT

public:

    enum NotificationPolicy
    {
        // Always send notification, unless the last event is still in the event queue
        NotificationPolicyDirect,
        // Always wait for a certain amount of time after the last event sent.
        // In particular, the first event will be sent only after waiting for this time span.
        // (Or no event will be sent, when the loading has finished before)
        // This is the default.
        NotificationPolicyTimeLimited
    };

    LoadSaveThread();
    // The thread will execute all pending tasks and wait for this upon destruction
    ~LoadSaveThread();

    // Append a task to load the given file to the task list
    void load(const QString& filePath);
    // Append a task to save the image to the task list
    void save(DImg &image, const QString& filePath, const char* format);

    void setNotificationPolicy(NotificationPolicy notificationPolicy);

signals:

    void signalImageLoaded(const QString& filePath, const DImg& img);
    void signalLoadingProgress(const QString& filePath, float progress);
    void signalImageSaved(const QString& filePath);
    void signalSavingProgress(const QString& filePath, float progress);

public:

    void imageLoaded(const QString& filePath, const DImg& img)
            { emit signalImageLoaded(filePath, img); };

    void loadingProgress(const QString& filePath, float progress)
            { emit signalLoadingProgress(filePath, progress); };

    void imageSaved(const QString& filePath)
            { emit signalImageSaved(filePath); };

    void savingProgress(const QString& filePath, float progress)
            { emit signalSavingProgress(filePath, progress); };

    virtual bool querySendNotifyEvent();

protected:

    virtual void run();
    virtual void customEvent(QCustomEvent *event);

    QMutex               m_mutex;

    QWaitCondition       m_condVar;

    QPtrList<class Task> m_todo;

    class Task          *m_currentTask;

    NotificationPolicy   m_notificationPolicy;

private:

    bool                 m_running;
    QTime                m_notificationTime;
    bool                 m_blockNotification;
};

class DIGIKAM_EXPORT ManagedLoadSaveThread : public LoadSaveThread
{

public:

    // Termination is controlled by setting the TerminationPolicy
    // Default is TerminationPolicyTerminateLoading
    ManagedLoadSaveThread();
    ~ManagedLoadSaveThread();

    enum LoadingPolicy
    {
        // Load image immediately, remove and stop all previous loading tasks.
        LoadingPolicyFirstRemovePrevious,
        // Prepend loading in front of all other tasks, but wait for the current task to finish.
        // No other tasks will be removed, preloading tasks will be stopped and postponed.
        LoadingPolicyPrepend,
        // Append loading task to the end of the list, but in front of all preloading tasks.
        // No other tasks will be removed, preloading tasks will be stopped and postponed.
        // This is similar to the simple load() operation from LoadSaveThread, except for the
        // special care taken for preloading.
        LoadingPolicyAppend,
        // Preload image, i.e. load it with low priority when no other tasks are scheduled.
        // All other tasks will take precedence, and preloading tasks will be stopped and
        // postponed when another task is added.
        // No progress info will be sent for preloaded images
        LoadingPolicyPreload
    };

    enum TerminationPolicy
    {
        // Wait for saving tasks, stop and remove loading tasks
        // This is the default.
        TerminationPolicyTerminateLoading,
        // Wait for loading and saving tasks, stop and remove preloading tasks
        TerminationPolicyTerminatePreloading,
        // Wait for all pending tasks
        TerminationPolicyWait
    };

    enum LoadingTaskFilter
    {
        // filter all loading tasks
        LoadingTaskFilterAll,
        // filter only tasks with preloading policy
        LoadingTaskFilterPreloading
    };

    // Append a task to load the given file to the task list.
    // If there is already a task for the given file, it will possibly be rescheduled,
    // but no second task will be added.
    // Only loading tasks will - if required by the policy - be stopped or removed,
    // saving tasks will not be touched.
    void load(const QString& filePath, LoadingPolicy policy = LoadingPolicyAppend);
    // Stop and remove tasks filtered by filePath and policy.
    // If filePath isNull, applies to all file paths.
    void stopLoading(const QString& filePath = QString(), LoadingTaskFilter filter = LoadingTaskFilterAll);

    // Append a task to save the image to the task list
    void save(DImg &image, const QString& filePath, const char* format);

    void setTerminationPolicy(TerminationPolicy terminationPolicy);

private:

    class LoadingTask *checkLoadingTask(class Task *task, LoadingTaskFilter filter);
    class LoadingTask *findExistingTask(const QString& filePath);
    void removeLoadingTasks(const QString& filePath, LoadingTaskFilter filter);

    TerminationPolicy m_terminationPolicy;
};

}      // namespace Digikam

#endif // LOAD_SAVE_THREAD_H
