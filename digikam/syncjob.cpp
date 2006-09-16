/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-10-04
 * Description :
 *
 * Copyright 2004 by Renchi Raju
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

#include <qapplication.h>
#include <qpixmap.h>

// KDE includes.

#include <kio/job.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes.

#include "albumsettings.h"
#include "syncjob.h"
#include "thumbnailjob.h"
#include "thumbnailsize.h"
#include "albumthumbnailloader.h"
#include "album.h"

void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

namespace Digikam
{

QString* SyncJob::lastErrorMsg_  = 0;
int      SyncJob::lastErrorCode_ = 0;

bool SyncJob::del(const KURL::List& urls, bool useTrash)
{
    SyncJob sj;

    if (useTrash)
        return sj.trashPriv(urls);
    else
        return sj.delPriv(urls);
}

bool SyncJob::file_move(const KURL &src, const KURL &dest)
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

SyncJob::SyncJob()
{
    thumbnail_ = 0;
    album_ = 0;
}

SyncJob::~SyncJob()
{
    if (thumbnail_)
        delete thumbnail_;
}

bool SyncJob::delPriv(const KURL::List& urls)
{
    success_ = true;

    KIO::Job* job = KIO::del( urls );
    connect( job, SIGNAL(result( KIO::Job* )),
             SLOT(slotResult( KIO::Job*)) );

    enter_loop();
    return success_;
}

bool SyncJob::trashPriv(const KURL::List& urls)
{
    success_ = true;
    KURL dest("trash:/");

    if (!KProtocolInfo::isKnownProtocol(dest))
    {
        dest = KGlobalSettings::trashPath();
    }

    KIO::Job* job = KIO::move( urls, dest );
    connect( job, SIGNAL(result( KIO::Job* )),
             SLOT(slotResult( KIO::Job*)) );

    enter_loop();
    return success_;
}

bool SyncJob::fileMovePriv(const KURL &src, const KURL &dest)
{
    success_ = true;

    KIO::FileCopyJob* job = KIO::file_move(src, dest, -1,
                                           true, false, false);
    connect( job, SIGNAL(result( KIO::Job* )),
             SLOT(slotResult( KIO::Job*)) );

    enter_loop();
    return success_;
}

void SyncJob::enter_loop()
{
    QWidget dummy(0,0,WType_Dialog | WShowModal);
    dummy.setFocusPolicy( QWidget::NoFocus );
    qt_enter_modal(&dummy);
    qApp->enter_loop();
    qt_leave_modal(&dummy);
}

void SyncJob::slotResult( KIO::Job * job )
{
    lastErrorCode_ = job->error();
    success_ = !(lastErrorCode_);
    if ( !success_ )
    {
        if ( !lastErrorMsg_ )
            lastErrorMsg_ = new QString;
        *lastErrorMsg_ = job->errorString();
    }
    qApp->exit_loop();
}

QPixmap SyncJob::getTagThumbnailPriv(TAlbum *album)
{
    if (thumbnail_)
        delete thumbnail_;
    thumbnail_ = new QPixmap;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    if (!loader->getTagThumbnail(album, *thumbnail_))
    {
        if (thumbnail_->isNull())
        {
            return loader->getStandardTagIcon(album);
        }
        else
        {
            return loader->blendIcons(loader->getStandardTagIcon(), *thumbnail_);
        }
    }
    else
    {
        connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
                this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

        connect(loader, SIGNAL(signalFailed(Album *)),
                this, SLOT(slotLoadThumbnailFailed(Album *)));

        album_ = album;
        enter_loop();
    }
    return *thumbnail_;
}

void SyncJob::slotLoadThumbnailFailed(Album *album)
{
    // TODO: setting _lastError*
    if (album == album_)
    {
        qApp->exit_loop();
    }
}

void SyncJob::slotGotThumbnailFromIcon(Album *album, const QPixmap& pix)
{
    if (album == album_)
    {
        *thumbnail_ = pix;
        qApp->exit_loop();
    }
}

QPixmap SyncJob::getTagThumbnailPriv(const QString &name, int size)
{
    thumbnailSize_ = size;
    if (thumbnail_)
        delete thumbnail_;
    thumbnail_ = new QPixmap;

    if(name.startsWith("/"))
    {
        ThumbnailJob *job = new ThumbnailJob(name,
                                             ThumbnailSize::Tiny,
                                             false,
                                             AlbumSettings::instance()->getExifRotate());
        connect(job,
                SIGNAL(signalThumbnail(const KURL&,
                                       const QPixmap&)),
                SLOT(slotGotThumbnailFromIcon(const KURL&,
                                              const QPixmap&)));
        connect(job,
                SIGNAL(signalFailed(const KURL&)),
                SLOT(slotLoadThumbnailFailed()));

        enter_loop();
        job->kill();
    }
    else
    {
        KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
        *thumbnail_ = iconLoader->loadIcon(name, KIcon::NoGroup, thumbnailSize_,
                                           KIcon::DefaultState, 0, true);
    }
    return *thumbnail_;
}

void SyncJob::slotLoadThumbnailFailed()
{
    // TODO: setting _lastError*
    qApp->exit_loop();
}

void SyncJob::slotGotThumbnailFromIcon(const KURL&, const QPixmap& pix)
{
    if(!pix.isNull() && (thumbnailSize_ < ThumbnailSize::Tiny))
    {
        int w1 = pix.width();
        int w2 = thumbnailSize_;
        int h1 = pix.height();
        int h2 = thumbnailSize_;
        thumbnail_->resize(w2,h2);
        bitBlt(thumbnail_, 0, 0, &pix, (w1-w2)/2, (h1-h2)/2, w2, h2);
    }
    else
    {
        *thumbnail_ = pix;
    }
    qApp->exit_loop();
}

QString SyncJob::lastErrorMsg()
{
    return (lastErrorMsg_ ? *lastErrorMsg_ : QString::null);
}

int SyncJob::lastErrorCode()
{
    return lastErrorCode_;
}

}  // namespace Digikam

#include "syncjob.moc"
