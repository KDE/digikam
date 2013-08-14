/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for thumbs generator.
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbstask.moc"

// KDE includes

#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>

// Local includes

#include "thumbnailloadthread.h"
#include "thumbnailsize.h"

namespace Digikam
{

class ThumbsTask::Private
{
public:

    Private()
    {
        cancel          = false;
        thumbLoadThread = ThumbnailLoadThread::defaultThread();
    }

    bool                 cancel;

    QString              path;
    ThumbnailLoadThread* thumbLoadThread;
};

// -------------------------------------------------------

ThumbsTask::ThumbsTask()
    : Job(0), d(new Private)
{
    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription, QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription, QPixmap)));
}

ThumbsTask::~ThumbsTask()
{
    slotCancel();
    delete d;
}

void ThumbsTask::setItem(const QString& path)
{
    d->path = path;
}

void ThumbsTask::slotCancel()
{
    d->cancel = true;
}

void ThumbsTask::run()
{
    if(d->cancel)
    {
        return;
    }

    d->thumbLoadThread->deleteThumbnail(d->path);
    d->thumbLoadThread->find(d->path);
}

void ThumbsTask::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (d->path != desc.filePath)
    {
        return;
    }

    emit signalFinished(pix);
}

}  // namespace Digikam
