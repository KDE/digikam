/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for thumbs generator.
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes
#include <QQueue>

// Local includes

#include "digikam_debug.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "maintenancedata.h"

namespace Digikam
{

class ThumbsTask::Private
{
public:

    Private()
        : catcher(0),
          data(0)
    {
    }

    ThumbnailImageCatcher* catcher;

    MaintenanceData*       data;
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
    cancel();

    d->catcher->setActive(false);
    d->catcher->thread()->stopAllTasks();

    delete d->catcher->thread();
    delete d->catcher;
    delete d;
}

void ThumbsTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void ThumbsTask::run()
{
    d->catcher->setActive(true);

    // While we have data (using this as check for non-null)
    while (d->data)
    {
        if (m_cancel)
        {
            d->catcher->setActive(false);
            d->catcher->thread()->stopAllTasks();
            return;
        }

        QString path = d->data->getImagePath();

        if (path.isEmpty())
        {
            break;
        }

        // TODO Should be improved by some update procedure
        d->catcher->thread()->deleteThumbnail(path);
        d->catcher->thread()->find(ThumbnailIdentifier(path));
        d->catcher->enqueue();
        QList<QImage> images = d->catcher->waitForThumbnails();
        emit signalFinished(images.first());
    }

    emit signalDone();

    d->catcher->setActive(false);
}

}  // namespace Digikam
