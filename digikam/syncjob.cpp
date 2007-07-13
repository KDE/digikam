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

QString* SyncJob::m_lastErrorMsg  = 0;
int      SyncJob::m_lastErrorCode = 0;

class SyncJobPriv
{
public:

    SyncJobPriv()
    {
        success     = false;
        album       = 0;
        thumbnail   = 0;
        waitingLoop = 0;
    }

    bool             success;
    
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

bool SyncJob::del(const KUrl::List& urls, bool useTrash)
{
    SyncJob sj;

    if (useTrash)
        return sj.trashPriv(urls);
    else
        return sj.delPriv(urls);
}

bool SyncJob::file_move(const KUrl &src, const KUrl &dest)
{
    SyncJob sj;
    return sj.fileMovePriv(src, dest);
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
    d->success = true;

    KIO::Job* job = KIO::del( urls );
    connect( job, SIGNAL(result( KIO::Job* )),
             this, SLOT(slotResult( KIO::Job*)) );

    enterWaitingLoop();
    return d->success;
}

bool SyncJob::trashPriv(const KUrl::List& urls)
{
    d->success = true;
    KIO::Job* job = KIO::trash( urls );
    connect( job, SIGNAL(result( KIO::Job* )),
             this, SLOT(slotResult( KIO::Job*)) );

    enterWaitingLoop();
    return d->success;
}

bool SyncJob::fileMovePriv(const KUrl &src, const KUrl &dest)
{
    d->success = true;

    KIO::FileCopyJob* job = KIO::file_move(src, dest, -1,
                                           true, false, false);
    connect( job, SIGNAL(result( KIO::Job* )),
             this, SLOT(slotResult( KIO::Job*)) );

    enterWaitingLoop();
    return d->success;
}

void SyncJob::slotResult( KIO::Job * job )
{
    m_lastErrorCode = job->error();
    d->success = !(m_lastErrorCode);
    if ( !d->success )
    {
        if ( !m_lastErrorMsg )
            m_lastErrorMsg = new QString;
        *m_lastErrorMsg = job->errorString();
    }
    quitWaitingLoop();
}

QPixmap SyncJob::getTagThumbnailPriv(TAlbum *album)
{
    if (d->thumbnail)
        delete d->thumbnail;

    d->thumbnail = new QPixmap();

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::componentData();

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
    // TODO: setting _lastError*
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
                                             AlbumSettings::componentData()->getExifRotate());

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
        *d->thumbnail = iconLoader->loadIcon(name, K3Icon::NoGroup, d->thumbnailSize,
                                           K3Icon::DefaultState, 0, true);
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

QString SyncJob::lastErrorMsg()
{
    return (m_lastErrorMsg ? *m_lastErrorMsg : QString());
}

int SyncJob::lastErrorCode()
{
    return m_lastErrorCode;
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
