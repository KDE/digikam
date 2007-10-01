/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-04
 * Description : sync IO jobs.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// Qt includes.

#include <QApplication>
#include <QPainter>
#include <QEventLoop>

// KDE includes.

#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/copyjob.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "albumsettings.h"
#include "thumbnailjob.h"
#include "thumbnailsize.h"
#include "albumthumbnailloader.h"
#include "album.h"
#include "syncjob.h"
#include "syncjob.moc"

namespace Digikam
{

class SyncJobPriv
{
public:

    SyncJobPriv()
    {
        result.success = false;
        album          = 0;
        thumbnail      = 0;
        waitingLoop    = 0;
    }

    SyncJobResult    result;

    int              thumbnailSize;

    QPixmap         *thumbnail;

    QEventLoop      *waitingLoop;

    Album           *album;
};

SyncJob::SyncJob()
{
    d = new SyncJobPriv;
    d->waitingLoop = new QEventLoop(this);
}

SyncJob::~SyncJob()
{
    if (d->thumbnail)
        delete d->thumbnail;

    d->thumbnail = 0;
    delete d;
}

SyncJobResult SyncJob::del(const KUrl::List& urls, bool useTrash)
{
    SyncJob sj;

    if (useTrash)
        sj.trashPriv(urls);
    else
        sj.delPriv(urls);

    return sj.d->result;
}

QPixmap SyncJob::getTagThumbnail(TAlbum *album)
{
    SyncJob sj;
    return sj.getTagThumbnailPriv(album);
}

QPixmap SyncJob::getTagThumbnail(const QString &name, int size)
{
    SyncJob sj;
    return sj.getTagThumbnailPriv(name, size);
}

bool SyncJob::delPriv(const KUrl::List& urls)
{
    KIO::Job* job = KIO::del( urls );
    connect( job, SIGNAL(result( KIO::Job* )),
             this, SLOT(slotResult( KIO::Job*)) );

    enterWaitingLoop();
    return d->result;
}

bool SyncJob::trashPriv(const KUrl::List& urls)
{
    KIO::Job* job = KIO::trash( urls );
    connect( job, SIGNAL(result( KIO::Job* )),
             this, SLOT(slotResult( KIO::Job*)) );

    enterWaitingLoop();
    return d->result;
}

void SyncJob::slotResult( KIO::Job * job )
{
    d->result.success = !(job->error());
    if ( !d->result )
    {
        d->result.errorString = job->errorString();
    }
    quitWaitingLoop();
}

QPixmap SyncJob::getTagThumbnailPriv(TAlbum *album)
{
    if (d->thumbnail)
        delete d->thumbnail;

    d->thumbnail = new QPixmap();

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

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
        connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
                this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

        connect(loader, SIGNAL(signalFailed(Album *)),
                this, SLOT(slotLoadThumbnailFailed(Album *)));

        d->album = album;
        enterWaitingLoop();
    }
    return *d->thumbnail;
}

void SyncJob::slotLoadThumbnailFailed(Album *album)
{
    if (album == d->album)
    {
        quitWaitingLoop();
    }
}

void SyncJob::slotGotThumbnailFromIcon(Album *album, const QPixmap& pix)
{
    if (album == d->album)
    {
        *d->thumbnail = pix;
        quitWaitingLoop();
    }
}

QPixmap SyncJob::getTagThumbnailPriv(const QString &name, int size)
{
    d->thumbnailSize = size;

    if (d->thumbnail)
        delete d->thumbnail;

    d->thumbnail = new QPixmap;

    if(name.startsWith("/"))
    {
        ThumbnailJob *job = new ThumbnailJob(name,
                                             ThumbnailSize::Tiny,
                                             false,
                                             AlbumSettings::instance()->getExifRotate());

        connect(job, SIGNAL(signalThumbnail(const KUrl&, const QPixmap&)),
                this, SLOT(slotGotThumbnailFromIcon(const KUrl&, const QPixmap&)));

        connect(job, SIGNAL(signalFailed(const KUrl&)),
                this, SLOT(slotLoadThumbnailFailed()));

        enterWaitingLoop();
        job->kill();
    }
    else
    {
        KIconLoader *iconLoader = KIconLoader::global();
        *d->thumbnail = iconLoader->loadIcon(name, KIconLoader::NoGroup, d->thumbnailSize);
    }
    return *d->thumbnail;
}

void SyncJob::slotLoadThumbnailFailed()
{
    // TODO: setting _lastError*
    quitWaitingLoop();
}

void SyncJob::slotGotThumbnailFromIcon(const KUrl&, const QPixmap& pix)
{
    if(!pix.isNull() && (d->thumbnailSize < ThumbnailSize::Tiny))
    {
        int w1 = pix.width();
        int w2 = d->thumbnailSize;
        int h1 = pix.height();
        int h2 = d->thumbnailSize;

        if (d->thumbnail)
            delete d->thumbnail;

        d->thumbnail = new QPixmap(w2, h2);
        QPainter p(d->thumbnail);
        p.drawPixmap(0, 0, pix, (w1-w2)/2, (h1-h2)/2, w2, h2);
        p.end();
    }
    else
    {
        *d->thumbnail = pix;
    }

    quitWaitingLoop();
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
