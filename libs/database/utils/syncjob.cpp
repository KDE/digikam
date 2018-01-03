/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-04
 * Description : synchronize Input/Output jobs.
 *
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "syncjob.h"

// Qt includes

#include <QEventLoop>

// Local includes

#include "applicationsettings.h"
#include "thumbnailsize.h"
#include "albumthumbnailloader.h"
#include "album.h"

namespace Digikam
{

class SyncJob::Private
{
public:

    Private() :
        waitingLoop(0),
        thumbnail(0),
        album(0)
    {
    }

    QEventLoop* waitingLoop;
    QPixmap*    thumbnail;

    Album*      album;
};

SyncJob::SyncJob()
    : d(new Private)
{
    d->waitingLoop = new QEventLoop(this);
}

SyncJob::~SyncJob()
{
    delete d->thumbnail;
    d->thumbnail = 0;
    delete d;
}


void SyncJob::enterWaitingLoop() const
{
    d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);
}

void SyncJob::quitWaitingLoop() const
{
    d->waitingLoop->quit();
}

// ---------------------------------------------------------------

QPixmap SyncJob::getTagThumbnail(TAlbum* const album)
{
    SyncJob sj;
    return sj.getTagThumbnailPriv(album);
}

QPixmap SyncJob::getTagThumbnailPriv(TAlbum* const album) const
{
    delete d->thumbnail;

    d->thumbnail                       = new QPixmap();
    AlbumThumbnailLoader* const loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album*,QPixmap)), this, SLOT(slotGotThumbnailFromIcon(Album*,QPixmap)),
            Qt::QueuedConnection);

    connect(loader, SIGNAL(signalFailed(Album*)), this, SLOT(slotLoadThumbnailFailed(Album*)),
            Qt::QueuedConnection);

    if (!loader->getTagThumbnail(album, *d->thumbnail))
    {
        if (d->thumbnail->isNull())
        {
            return loader->getStandardTagIcon(album);
        }
    }
    else
    {
        d->album = album;
        enterWaitingLoop();
    }

    return *d->thumbnail;
}

void SyncJob::slotLoadThumbnailFailed(Album* album)
{
    if (album == d->album)
    {
        quitWaitingLoop();
    }
}

void SyncJob::slotGotThumbnailFromIcon(Album* album, const QPixmap& pix)
{
    if (album == d->album)
    {
        *d->thumbnail = pix;
        quitWaitingLoop();
    }
}

} // namespace Digikam
