/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * Copyright (C) 2005-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "loadsavethread.h"
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


// This is a hack needed to prevent hanging when a KProcess-based loader (raw loader)
// is waiting for the process to finish, but the main thread is waiting
// for the thread to finish and no KProcess events are delivered.
// Remove when porting to Qt4.
bool LoadSaveThread::isShuttingDown()
{
    // the condition is met after d->running is set to false in the destructor
    return running() && !d->running;
}

bool LoadSaveThread::exifRotate(DImg &image, const QString& filePath)
{
    QVariant attribute(image.attribute("exifRotated"));
    if (attribute.isValid() && attribute.toBool())
        return false;

    // Raw files are already rotated properlly by dcraw. Only perform auto-rotation with JPEG/PNG/TIFF file.
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

    /*
    if (orientation == DMetadata::ORIENTATION_NORMAL ||
        orientation == DMetadata::ORIENTATION_UNSPECIFIED)
        return;

    QWMatrix matrix;

    switch (orientation)
    {
        case DMetadata::ORIENTATION_NORMAL:
        case DMetadata::ORIENTATION_UNSPECIFIED:
            break;

        case DMetadata::ORIENTATION_HFLIP:
            matrix.scale(-1, 1);
            break;

        case DMetadata::ORIENTATION_ROT_180:
            matrix.rotate(180);
            break;

        case DMetadata::ORIENTATION_VFLIP:
            matrix.scale(1, -1);
            break;

        case DMetadata::ORIENTATION_ROT_90_HFLIP:
            matrix.scale(-1, 1);
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_90:
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_90_VFLIP:
            matrix.scale(1, -1);
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_270:
            matrix.rotate(270);
            break;
    }

    // transform accordingly
    thumb = thumb.xForm( matrix );
    */
}




}   // namespace Digikam

#include "loadsavethread.moc"
