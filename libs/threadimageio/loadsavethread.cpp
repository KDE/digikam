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

// KDE includes

#include <kdebug.h>

// Local includes.

#include "loadsavethread.h"
#include "managedloadsavethread.h"
#include "sharedloadsavethread.h"
#include "loadingcache.h"
#include "loadsavetask.h"

namespace Digikam
{

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
        lastTask          = 0;
    }

    bool  running;
    bool  blockNotification;
    LoadSaveTask *lastTask;

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

    if (d->lastTask)
        delete d->lastTask;
    delete d;
}

void LoadSaveThread::load(LoadingDescription description)
{
    QMutexLocker lock(&m_mutex);
    m_todo.append(new LoadingTask(this, description));
    m_condVar.wakeAll();
}

void LoadSaveThread::save(DImg &image, const QString& filePath, const QString &format)
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
            if (d->lastTask)
            {
                delete d->lastTask;
                d->lastTask = 0;
            }
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

void LoadSaveThread::taskHasFinished()
{
    // This function is called by the tasks before they send their final message.
    // This is to guarantee the user of the API that at least the final message
    // is sent after load() has been called. This might not been the case
    // if m_currentTask is currently loading the same image and a race condition
    // between the return from execute and the next run of the loop above occurs.
    QMutexLocker lock(&m_mutex);
    d->lastTask = m_currentTask;
    m_currentTask = 0;
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


}   // namespace Digikam

#include "loadsavethread.moc"
