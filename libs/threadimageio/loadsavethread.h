/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "loadingdescription.h"

namespace Digikam
{

class LoadSaveThreadPriv;
class LoadSaveTask;

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

    // used by SharedLoadSaveThread only
    enum AccessMode
    {
        // image will only be used for reading
        AccessModeRead,
        // image data will possibly be changed
        AccessModeReadWrite
    };

    LoadSaveThread();
    // The thread will execute all pending tasks and wait for this upon destruction
    ~LoadSaveThread();

    // Append a task to load the given file to the task list
    void load(LoadingDescription description);
    // Append a task to save the image to the task list
    void save(DImg &image, const QString& filePath, const QString &format);

    void setNotificationPolicy(NotificationPolicy notificationPolicy);

    bool isShuttingDown();

signals:

    // This signal is emitted when the loading process begins.
    void signalImageStartedLoading(const LoadingDescription &loadingDescription);
    // This signal is emitted whenever new progress info is available
    // and the notification policy allows emitting the signal.
    // No progress info will be sent for preloaded images (ManagedLoadSaveThread).
    void signalLoadingProgress(const LoadingDescription &loadingDescription, float progress);
    // This signal is emitted when the loading process has finished.
    // If the process failed, img is null.
    void signalImageLoaded(const LoadingDescription &loadingDescription, const DImg& img);
    // This signal is emitted if
    //  - you are doing shared loading (SharedLoadSaveThread)
    //  - you started a loading operation with a LoadingDescription for
    //    a reduced version of the image
    //  - another thread started a loading operation for a more complete version
    // You may want to cancel the current operation and start with the given loadingDescription
    void signalMoreCompleteLoadingAvailable(const LoadingDescription &oldLoadingDescription,
                                            const LoadingDescription &newLoadingDescription);

    void signalImageStartedSaving(const QString& filePath);
    void signalSavingProgress(const QString& filePath, float progress);
    void signalImageSaved(const QString& filePath, bool success);

public:

    void imageStartedLoading(const LoadingDescription &loadingDescription)
            { emit signalImageStartedLoading(loadingDescription); };

    void loadingProgress(const LoadingDescription &loadingDescription, float progress)
            { emit signalLoadingProgress(loadingDescription, progress); };

    void imageLoaded(const LoadingDescription &loadingDescription, const DImg& img)
            { emit signalImageLoaded(loadingDescription, img); };

    void moreCompleteLoadingAvailable(const LoadingDescription &oldLoadingDescription,
                                      const LoadingDescription &newLoadingDescription)
            { emit signalMoreCompleteLoadingAvailable(oldLoadingDescription, newLoadingDescription); }

    void imageStartedSaving(const QString& filePath)
            { emit signalImageStartedSaving(filePath); };

    void savingProgress(const QString& filePath, float progress)
            { emit signalSavingProgress(filePath, progress); };

    void imageSaved(const QString& filePath, bool success)
            { emit signalImageSaved(filePath, success); };

    virtual bool querySendNotifyEvent();
    virtual void taskHasFinished();

protected:

    virtual void run();
    virtual void customEvent(QCustomEvent *event);

    QMutex               m_mutex;

    QWaitCondition       m_condVar;

    QPtrList<LoadSaveTask> m_todo;

    LoadSaveTask        *m_currentTask;

    NotificationPolicy   m_notificationPolicy;

private:

    LoadSaveThreadPriv* d;

};

}      // namespace Digikam

#endif // LOAD_SAVE_THREAD_H
