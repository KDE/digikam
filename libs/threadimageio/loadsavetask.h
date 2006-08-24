/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-20
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


#ifndef LOAD_SAVE_TASK_H
#define LOAD_SAVE_TASK_H

// Qt includes.

#include <qevent.h>

// Local includes.

#include "dimg.h"
#include "dimgloaderobserver.h"
#include "loadingdescription.h"
#include "loadingcache.h"

namespace Digikam
{

class LoadSaveThread;

class LoadSaveTask
{
public:

    LoadSaveTask(LoadSaveThread* thread)
        : m_thread(thread)
        {};
    virtual ~LoadSaveTask() {};

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

    ProgressEvent(float progress)
        : m_progress(progress)
        {};

protected:

    float m_progress;
};

//---------------------------------------------------------------------------------------------------

class LoadingProgressEvent : public ProgressEvent
{
public:

    LoadingProgressEvent(const LoadingDescription &loadingDescription, float progress)
        : ProgressEvent(progress),
          m_loadingDescription(loadingDescription)
        {};

    virtual void notify(LoadSaveThread *thread);

private:

    LoadingDescription m_loadingDescription;
};

//---------------------------------------------------------------------------------------------------

class SavingProgressEvent : public ProgressEvent
{
public:

    SavingProgressEvent(const QString& filePath, float progress)
        : ProgressEvent(progress),
          m_filePath(filePath)
        {};

    virtual void notify(LoadSaveThread *thread);

private:

    QString m_filePath;
};

//---------------------------------------------------------------------------------------------------

class StartedLoadingEvent : public NotifyEvent
{
public:

    StartedLoadingEvent(const LoadingDescription &loadingDescription)
        : m_loadingDescription(loadingDescription)
        {};

    virtual void notify(LoadSaveThread *thread);

private:

    LoadingDescription m_loadingDescription;
};

//---------------------------------------------------------------------------------------------------

class StartedSavingEvent : public NotifyEvent
{
public:

    StartedSavingEvent(const QString& filePath)
    : m_filePath(filePath)
    {};

    virtual void notify(LoadSaveThread *thread);

private:

    QString m_filePath;
};

//---------------------------------------------------------------------------------------------------

class LoadedEvent : public NotifyEvent
{
public:

    LoadedEvent(const LoadingDescription &loadingDescription, DImg &img)
        : m_loadingDescription(loadingDescription), m_img(img)
        {};

    virtual void notify(LoadSaveThread *thread);

private:

    LoadingDescription m_loadingDescription;
    DImg    m_img;
};

//---------------------------------------------------------------------------------------------------

class MoreCompleteLoadingAvailableEvent : public NotifyEvent
{
public:

    MoreCompleteLoadingAvailableEvent(const LoadingDescription &oldLoadingDescription,
                                      const LoadingDescription &newLoadingDescription)
        : m_oldDescription(oldLoadingDescription), m_newDescription(newLoadingDescription)
        {};

    virtual void notify(LoadSaveThread *thread);

private:

    LoadingDescription m_oldDescription;
    LoadingDescription m_newDescription;
};

//---------------------------------------------------------------------------------------------------

class LoadingTask : public LoadSaveTask, public DImgLoaderObserver
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
        : LoadSaveTask(thread), m_loadingDescription(description), m_loadingTaskStatus(loadingTaskStatus)
        {}

    // LoadSaveTask

    virtual void execute();
    virtual TaskType type();

    // DImgLoaderObserver

    virtual void progressInfo(const DImg *, float progress);
    virtual bool continueQuery(const DImg *);
    virtual bool isShuttingDown();

    virtual void setStatus(LoadingTaskStatus status);

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
          m_accessMode(mode), m_completed(false), m_usedProcess(0)
        {}

    virtual void execute();
    virtual void progressInfo(const DImg *, float progress);
    virtual bool continueQuery(const DImg *);
    virtual void setStatus(LoadingTaskStatus status);

    // LoadingProcess

    virtual bool completed();
    virtual QString filePath();
    virtual QString cacheKey();
    virtual void addListener(LoadingProcessListener *listener);
    virtual void removeListener(LoadingProcessListener *listener);
    virtual void notifyNewLoadingProcess(LoadingProcess *process, LoadingDescription description);

    // LoadingProcessListener

    virtual bool querySendNotifyEvent();
    virtual QObject *eventReceiver();
    virtual LoadSaveThread::AccessMode accessMode();

private:

    LoadSaveThread::AccessMode m_accessMode;
    bool m_completed;
    LoadingProcess *m_usedProcess;
    QPtrList<LoadingProcessListener> m_listeners;
};

//---------------------------------------------------------------------------------------------------

class SavedEvent : public NotifyEvent
{
public:

    SavedEvent(const QString &filePath, bool success)
        : m_filePath(filePath), m_success(success)
        {};

    virtual void notify(LoadSaveThread *thread);

private:

    QString m_filePath;
    bool m_success;
};

//---------------------------------------------------------------------------------------------------

class SavingTask : public LoadSaveTask, public DImgLoaderObserver
{
public:

    enum SavingTaskStatus
    {
        SavingTaskStatusSaving,
        SavingTaskStatusStopping
    };

    SavingTask(LoadSaveThread* thread, DImg &img, const QString &filePath, const QString &format)
        : LoadSaveTask(thread), m_img(img), m_filePath(filePath), m_format(format)
        {};

    virtual void execute();
    virtual TaskType type();

    virtual void progressInfo(const DImg *, float progress);
    virtual bool continueQuery(const DImg *);

    virtual void setStatus(SavingTaskStatus status);

    SavingTaskStatus status() const
    {
        return m_savingTaskStatus;
    }

    QString filePath() const
    {
        return m_filePath;
    }

private:

    DImg        m_img;
    QString     m_filePath;
    QString     m_format;
    SavingTaskStatus m_savingTaskStatus;
};

}   // namespace Digikam

#endif // LOAD_SAVE_TASK_H
