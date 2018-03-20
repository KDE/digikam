/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : a class to manage video thumbnails extraction
 *
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016-2018 by Maik Qualmann <metzpinguin at gmail dot com>
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
#include "filmstripfilter.h"
#include "thumbnailsize.h"
#include "digikam_debug.h"

namespace Digikam
{

class VideoThumbnailerJob::Private
{
public:

    explicit Private()
    {
        canceled    = false;
        running     = false;
        createStrip = true;
        exifRotate  = true;
        thumbSize   = ThumbnailSize::Huge;
    }

    volatile bool     canceled;
    volatile bool     running;
    bool              createStrip;
    bool              exifRotate;
    int               thumbSize;

    QMutex            mutex;
    QWaitCondition    condVar;

    QStringList       todo;
};

VideoThumbnailerJob::VideoThumbnailerJob(QObject* const parent)
    : QThread(parent),
      d(new Private)
{
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

    d->condVar.wakeAll();
}

void VideoThumbnailerJob::run()
{
    while (d->running)
    {
        QMutexLocker lock(&d->mutex);

        if (!d->todo.isEmpty())
        {
            QString file = d->todo.takeFirst();
            qCDebug(DIGIKAM_GENERAL_LOG) << "Request to get thumbnail for" << file;

            VideoThumbnailer thumbnailer;
            FilmStripFilter  filmStrip;
            QImage           img;

            if (d->createStrip)
            {
                thumbnailer.addFilter(&filmStrip);
            }

            thumbnailer.setThumbnailSize(d->thumbSize);
            thumbnailer.generateThumbnail(file, img);

            if (!img.isNull())
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Video thumbnail extracted for" << file << "with size:" << img.size();
                emit signalThumbnailDone(file, img);
            }
            else
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to extract video thumbnail for" << file;
                emit signalThumbnailFailed(file);
            }
        }
        else
        {
            emit signalThumbnailJobFinished();
            d->condVar.wait(&d->mutex);
        }
    }
}

}  // namespace Digikam
