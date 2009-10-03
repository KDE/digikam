/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2005-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "loadsavethread.h"
#include "loadsavethread.moc"

// KDE includes



// Local includes

#include "dmetadata.h"
#include "managedloadsavethread.h"
#include "sharedloadsavethread.h"
#include "loadsavetask.h"

namespace Digikam
{

class LoadSaveThreadPriv
{
public:

    LoadSaveThreadPriv()
    {
        running           = true;
        blockNotification = false;
        lastTask          = 0;
    }

    bool          running;
    bool          blockNotification;

    QTime         notificationTime;

    LoadSaveTask *lastTask;
};

//---------------------------------------------------------------------------------------------------

LoadSaveThread::LoadSaveThread()
              : d(new LoadSaveThreadPriv)
{
    m_currentTask        = 0;
    m_notificationPolicy = NotificationPolicyTimeLimited;

    start();
}

LoadSaveThread::~LoadSaveThread()
{
    shutdownThread();

    if (d->lastTask)
        delete d->lastTask;
    foreach (LoadSaveTask *task, m_todo)
        delete task;
    delete d;
}

void LoadSaveThread::shutdownThread()
{
    d->running = false;
    {
        QMutexLocker lock(&m_mutex);
        m_condVar.wakeAll();
    }

    wait();
}

void LoadSaveThread::load(LoadingDescription description)
{
    QMutexLocker lock(&m_mutex);
    m_todo << new LoadingTask(this, description);
    m_condVar.wakeAll();
}

void LoadSaveThread::save(DImg& image, const QString& filePath, const QString& format)
{
    QMutexLocker lock(&m_mutex);
    m_todo << new SavingTask(this, image, filePath, format);
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
            if (m_currentTask)
            {
                delete m_currentTask;
                m_currentTask = 0;
            }
            if (!m_todo.isEmpty())
            {
                m_currentTask = m_todo.takeFirst();
                if (m_notificationPolicy == NotificationPolicyTimeLimited)
                {
                    // set timing values so that first event is sent only
                    // after an initial time span.
                    d->notificationTime = QTime::currentTime();
                    d->blockNotification = true;
                }
            }
            else
                m_condVar.wait(&m_mutex);
        }
        if (m_currentTask)
            m_currentTask->execute();
    }
}

void LoadSaveThread::taskHasFinished()
{
    // This function is called by the tasks _before_ they send their _final_ message.
    // This is to guarantee the user of the API that at least the final message
    // is sent after load() has been called.
    // We set m_currentTask to 0 here. If a new task is appended, base classes usually check
    // that m_currentTask is not currently loading the same task.
    // Now it might happen that m_currentTask has already emitted its final signal,
    // but the new task is rejected afterwards when m_currentTask is still the task
    // that has actually already finished (execute() in the loop above is of course not under mutex).
    // So we set m_currentTask to 0 immediately before the final message is emitted,
    // so that anyone who finds this task running as m_current task will get a message.
    QMutexLocker lock(&m_mutex);
    d->lastTask = m_currentTask;
    m_currentTask = 0;
}

void LoadSaveThread::imageStartedLoading(const LoadingDescription& loadingDescription)
{
    notificationReceived();
    emit signalImageStartedLoading(loadingDescription);
}

void LoadSaveThread::loadingProgress(const LoadingDescription& loadingDescription, float progress)
{
    notificationReceived();
    emit signalLoadingProgress(loadingDescription, progress);
}

void LoadSaveThread::imageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    notificationReceived();
    emit signalImageLoaded(loadingDescription, img);
}

void LoadSaveThread::moreCompleteLoadingAvailable(const LoadingDescription& oldLoadingDescription,
                          const LoadingDescription& newLoadingDescription)
{
    notificationReceived();
    emit signalMoreCompleteLoadingAvailable(oldLoadingDescription, newLoadingDescription);
}

void LoadSaveThread::imageStartedSaving(const QString& filePath)
{
    notificationReceived();
    emit signalImageStartedSaving(filePath);
}

void LoadSaveThread::savingProgress(const QString& filePath, float progress)
{
    notificationReceived();
    emit signalSavingProgress(filePath, progress);
}

void LoadSaveThread::imageSaved(const QString& filePath, bool success)
{
    notificationReceived();
    emit signalImageSaved(filePath, success);
}

void LoadSaveThread::thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img)
{
    notificationReceived();
    emit signalThumbnailLoaded(loadingDescription, img);
}

void LoadSaveThread::notificationReceived()
{
    switch (m_notificationPolicy)
    {
        case NotificationPolicyDirect:
            d->blockNotification = false;
            break;
        case NotificationPolicyTimeLimited:
            break;
    }
}

void LoadSaveThread::setNotificationPolicy(NotificationPolicy notificationPolicy)
{
    m_notificationPolicy = notificationPolicy;
    d->blockNotification = false;
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
                d->notificationTime  = QTime::currentTime();
                d->blockNotification = true;
                return true;
            }
            break;
    }
    return false;
}


// This is a hack needed to prevent hanging when a K3Process-based loader (raw loader)
// is waiting for the process to finish, but the main thread is waiting
// for the thread to finish and no K3Process events are delivered.
// Remove when porting to Qt4.
bool LoadSaveThread::isShuttingDown()
{
    // the condition is met after d->running is set to false in the destructor
    return isRunning() && !d->running;
}

bool LoadSaveThread::exifRotate(DImg& image, const QString& filePath)
{
    // Keep in sync with the variant in thumbnailcreator.cpp
    QVariant attribute(image.attribute("exifRotated"));
    if (attribute.isValid() && attribute.toBool())
        return false;

    // Raw files are already rotated properly by dcraw. Only perform auto-rotation with JPEG/PNG/TIFF file.
    // We don't have a feedback from dcraw about auto-rotated RAW file during decoding. Return true anyway.

    attribute = image.attribute("fromRawEmbeddedPreview");
    if (DImg::fileFormat(filePath) == DImg::RAW && !(attribute.isValid() && attribute.toBool()) )
    {
        return true;
    }

    // Rotate thumbnail based on metadata orientation information

    DMetadata metadata(filePath);
    DMetadata::ImageOrientation orientation = metadata.getImageOrientation();

    bool rotatedOrFlipped = false;

    if(orientation != DMetadata::ORIENTATION_NORMAL)
    {
        switch (orientation)
        {
            case DMetadata::ORIENTATION_NORMAL:
            case DMetadata::ORIENTATION_UNSPECIFIED:
                break;

            case DMetadata::ORIENTATION_HFLIP:
                image.flip(DImg::HORIZONTAL);
                rotatedOrFlipped = true;
                break;

            case DMetadata::ORIENTATION_ROT_180:
                image.rotate(DImg::ROT180);
                rotatedOrFlipped = true;
                break;

            case DMetadata::ORIENTATION_VFLIP:
                image.flip(DImg::VERTICAL);
                rotatedOrFlipped = true;
                break;

            case DMetadata::ORIENTATION_ROT_90_HFLIP:
                image.rotate(DImg::ROT90);
                image.flip(DImg::HORIZONTAL);
                rotatedOrFlipped = true;
                break;

            case DMetadata::ORIENTATION_ROT_90:
                image.rotate(DImg::ROT90);
                rotatedOrFlipped = true;
                break;

            case DMetadata::ORIENTATION_ROT_90_VFLIP:
                image.rotate(DImg::ROT90);
                image.flip(DImg::VERTICAL);
                rotatedOrFlipped = true;
                break;

            case DMetadata::ORIENTATION_ROT_270:
                image.rotate(DImg::ROT270);
                rotatedOrFlipped = true;
                break;
        }
    }

    image.setAttribute("exifRotated", true);
    return rotatedOrFlipped;
}

}   // namespace Digikam
