/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : a class to manage video thumbnails extraction
 *
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
        jobDone     = true;
        createStrip = true;
        exifRotate  = true;
        thumbSize   = ThumbnailSize::Huge;
        vthumb      = 0;
    }

    volatile bool     canceled;
    volatile bool     running;
    volatile bool     jobDone;
    bool              createStrip;
    bool              exifRotate;
    int               thumbSize;

    QMutex            mutex;
    QWaitCondition    condVar;

    QStringList       todo;
    QString           currentFile;
    VideoThumbnailer* vthumb;
};

VideoThumbnailerJob::VideoThumbnailerJob(QObject* const parent)
    : QThread(parent),
      d(new Private)
{
    d->vthumb = new VideoThumbnailer(this);

    connect(this, SIGNAL(signalGetThumbnail(QString,int,bool,bool)),
            d->vthumb, SLOT(slotGetThumbnail(QString,int,bool,bool)));

    connect(d->vthumb, SIGNAL(signalThumbnailDone(QString,QImage)),
            this, SLOT(slotThumbnailDone(QString,QImage)));

    connect(d->vthumb, SIGNAL(signalThumbnailFailed(QString)),
            this, SLOT(slotThumbnailFailed(QString)));
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

void VideoThumbnailerJob::setExifRotate(bool rotate)
{
    d->exifRotate = rotate;
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

        if (d->jobDone && !d->todo.isEmpty())
        {
            d->jobDone = false;
            d->currentFile = d->todo.takeFirst();
            qCDebug(DIGIKAM_GENERAL_LOG) << "Request to get thumbnail for " << d->currentFile;
            emit signalGetThumbnail(d->currentFile, d->thumbSize, d->createStrip, d->exifRotate);
        }

        d->condVar.wait(&d->mutex);
    }
}

void VideoThumbnailerJob::slotThumbnailDone(const QString& file, const QImage& img)
{
    if (d->jobDone || d->currentFile != file)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Video thumbnail extracted for " << file << " :: " << img;
    emit signalThumbnailDone(file, img);
    d->jobDone = true;
    processOne();
}

void VideoThumbnailerJob::slotThumbnailFailed(const QString& file)
{
    if (d->jobDone || d->currentFile != file)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to extract video thumbnail for " << file;
    emit signalThumbnailFailed(file);
    d->jobDone = true;
    processOne();
}

}  // namespace Digikam
