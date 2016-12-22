/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for thumbs generator.
 *
 * Copyright (C) 2013-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbstask.h"

// Local includes

#include "digikam_debug.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"

namespace Digikam
{

class ThumbsTask::Private
{
public:

    Private()
    {
        cancel  = false;
        catcher = 0;
    }

    QString                path;

    bool                   cancel;

    ThumbnailImageCatcher* catcher;
};

// -------------------------------------------------------

ThumbsTask::ThumbsTask()
    : ActionJob(),
      d(new Private)
{
    ThumbnailLoadThread* const thread = new ThumbnailLoadThread;
    thread->setPixmapRequested(false);
    thread->setThumbnailSize(ThumbnailLoadThread::maximumThumbnailSize());
    d->catcher                        = new ThumbnailImageCatcher(thread, this);
}

ThumbsTask::~ThumbsTask()
{
    slotCancel();

    delete d->catcher->thread();
    delete d->catcher;
    delete d;
}

void ThumbsTask::setItem(const QString& path)
{
    d->path = path;
    d->catcher->thread()->deleteThumbnail(d->path);
}

void ThumbsTask::slotCancel()
{
    d->catcher->thread()->stopAllTasks();
    d->catcher->cancel();
    d->cancel = true;
}

void ThumbsTask::run()
{
    if (d->cancel)
    {
        return;
    }

    d->catcher->setActive(true);

    d->catcher->thread()->find(ThumbnailIdentifier(d->path));
    d->catcher->enqueue();
    QList<QImage> images = d->catcher->waitForThumbnails();
    emit signalFinished(images.first());
    emit signalDone();

    d->catcher->setActive(false);
}

}  // namespace Digikam
