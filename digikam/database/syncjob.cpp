/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-04
 * Description : synchronize Input/Output jobs.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Concept copied from kdelibs/kio/kio/netaccess.h/cpp
 *   This file is part of the KDE libraries
 *   Copyright (C) 1997 Torben Weis (weis@kde.org)
 *   Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
 *   Copyright (C) 1999 David Faure (faure@kde.org)
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

#include "syncjob.moc"

// Qt includes

#include <QApplication>
#include <QPainter>
#include <QEventLoop>

// KDE includes

#include <kjob.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/copyjob.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kapplication.h>

// Local includes

#include "albumsettings.h"
#include "thumbnailsize.h"
#include "albumthumbnailloader.h"
#include "album.h"

namespace Digikam
{

class SyncJob::SyncJobPriv
{
public:

    SyncJobPriv() :
        thumbnailSize(0),
        thumbnail(0),
        waitingLoop(0),
        album(0)
    {
        result.success = false;
    }

    SyncJobResult result;

    int           thumbnailSize;

    QPixmap*      thumbnail;
    QEventLoop*   waitingLoop;

    Album*        album;
};

SyncJob::SyncJob()
    : d(new SyncJobPriv)
{
    d->waitingLoop = new QEventLoop(this);
}

SyncJob::~SyncJob()
{
    delete d->thumbnail;
    d->thumbnail = 0;
    delete d;
}

SyncJobResult SyncJob::del(const KUrl::List& urls, bool useTrash)
{
    SyncJob sj;

    if (useTrash)
    {
        sj.trashPriv(urls);
    }
    else
    {
        sj.delPriv(urls);
    }

    return sj.d->result;
}

QPixmap SyncJob::getTagThumbnail(TAlbum* album)
{
    SyncJob sj;
    return sj.getTagThumbnailPriv(album);
}

bool SyncJob::delPriv(const KUrl::List& urls)
{
    KIO::Job* job = KIO::del( urls );

    connect( job, SIGNAL(result( KJob* )),
             this, SLOT(slotResult( KJob* )) );

    enterWaitingLoop();
    return d->result;
}

bool SyncJob::trashPriv(const KUrl::List& urls)
{
    KIO::Job* job = KIO::trash( urls );

    connect( job, SIGNAL(result( KJob* )),
             this, SLOT(slotResult( KJob* )) );

    enterWaitingLoop();
    return d->result;
}

void SyncJob::slotResult(KJob* job)
{
    d->result.success = !(job->error());

    if ( !d->result )
    {
        d->result.errorString = job->errorString();
    }

    quitWaitingLoop();
}

QPixmap SyncJob::getTagThumbnailPriv(TAlbum* album)
{
    delete d->thumbnail;

    d->thumbnail                 = new QPixmap();
    AlbumThumbnailLoader* loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album*, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album*, const QPixmap&)),
            Qt::QueuedConnection);

    connect(loader, SIGNAL(signalFailed(Album*)),
            this, SLOT(slotLoadThumbnailFailed(Album*)),
            Qt::QueuedConnection);

    if (!loader->getTagThumbnail(album, *d->thumbnail))
    {
        if (d->thumbnail->isNull())
        {
            return loader->getStandardTagIcon(album);
        }
        else
        {
            return loader->blendIcons(loader->getStandardTagIcon(), *d->thumbnail);
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

void SyncJob::enterWaitingLoop()
{
    d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);
}

void SyncJob::quitWaitingLoop()
{
    d->waitingLoop->quit();
}

}  // namespace Digikam
