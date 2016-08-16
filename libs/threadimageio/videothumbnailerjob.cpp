/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : a class to manage video thumbnails extraction
 *
 * Copyright (C) 2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "videothumbnailerjob.h"

// Qt includes

#include <QImage>
#include <QMutex>
#include <QWaitCondition>

// Local includes

#include "videothumbnailer.h"
#include "thumbnailsize.h"
#include "digikam_debug.h"

namespace Digikam
{

class VideoThumbnailerJob::Private
{
public:

    Private()
    {
        canceled    = false;
        running     = false;
        createStrip = true;
        thumbSize   = ThumbnailSize::Huge;
        thumbJob    = 0;
        vthumb      = 0;
    }

    volatile bool     canceled;
    volatile bool     running;

    bool              createStrip;
    int               thumbSize;
    quintptr          thumbJob;

    QMutex            mutex;
    QWaitCondition    condVar;

    QStringList       todo;
    VideoThumbnailer* vthumb;
};

VideoThumbnailerJob::VideoThumbnailerJob(QObject* const parent)
    : QThread(parent),
      d(new Private)
{
    d->thumbJob = reinterpret_cast<quintptr>(this);
    d->vthumb   = VideoThumbnailer::instance();

    connect(d->vthumb, SIGNAL(signalThumbnailDone(quintptr, const QString&, const QImage&)),
            this, SLOT(slotThumbnailDone(quintptr, const QString&, const QImage&)));

    connect(d->vthumb, SIGNAL(signalThumbnailFailed(quintptr, const QString&)),
            this, SLOT(slotThumbnailFailed(quintptr, const QString&)));
}

VideoThumbnailerJob::~VideoThumbnailerJob()
{
    // clear updateItems, stop processing
    slotCancel();

    // stop thread
    {
        QMutexLocker lock(&d->mutex);
        d->running = false;
        d->condVar.wakeAll();
    }

    wait();

    delete d;
}

void VideoThumbnailerJob::setThumbnailSize(int size)
{
    d->thumbSize = size;
}

void VideoThumbnailerJob::setCreateStrip(bool strip)
{
    d->createStrip = strip;
}

void VideoThumbnailerJob::slotCancel()
{
    QMutexLocker lock(&d->mutex);
    d->running = false;
    d->todo.clear();
}

void VideoThumbnailerJob::addItems(const QStringList& files)
{
    if (files.isEmpty())
    {
        return;
    }

    {
        QMutexLocker lock(&d->mutex);
        d->running = true;
        d->todo << files;

        if (!isRunning())
        {
            start(LowPriority);
        }
    }

    processOne();
}

void VideoThumbnailerJob::processOne()
{
    if (!d->todo.isEmpty())
    {
        d->condVar.wakeAll();
    }
    else
    {
        emit signalThumbnailJobFinished();
    }
}

void VideoThumbnailerJob::run()
{
    while (d->running)
    {
        QMutexLocker lock(&d->mutex);

        if (d->vthumb->isReady() && !d->todo.isEmpty())
        {
            QString file = d->todo.takeFirst();
            qCDebug(DIGIKAM_GENERAL_LOG) << "Request to get thumbnail for " << file;

            if (!d->vthumb->getThumbnail(d->thumbJob, file, d->thumbSize, d->createStrip))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to extract video thumbnail for " << file;
                emit signalThumbnailFailed(file);

                if (d->todo.isEmpty())
                {
                    emit signalThumbnailJobFinished();
                }
            }
        }

        if (d->todo.isEmpty())
        {
            d->condVar.wait(&d->mutex);
        }
        else
        {
            d->condVar.wait(&d->mutex, 250);
        }
    }
}

void VideoThumbnailerJob::slotThumbnailDone(quintptr job, const QString& file, const QImage& img)
{
    if (d->thumbJob != job)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Video thumbnail extracted for " << file << " :: " << img;
    emit signalThumbnailDone(file, img);
    processOne();
}

void VideoThumbnailerJob::slotThumbnailFailed(quintptr job, const QString& file)
{
    if (d->thumbJob != job)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to extract video thumbnail for " << file;
    emit signalThumbnailFailed(file);
    processOne();
}

}  // namespace Digikam
