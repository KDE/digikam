/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-14
 * Description : a pixmap manager for album icon view.  
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// Qt includes.

#include <QCache>
#include <QPointer>
#include <QPixmap>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QImage>

// KDE includes.

#include <kcodecs.h>
#include <kiconloader.h>
#include <kurl.h>

// Local includes.

#include "thumbnailjob.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumsettings.h"
#include "pixmapmanager.h"
#include "pixmapmanager.moc"

/** @file pixmapmanager.cpp */

namespace Digikam
{

class PixmapManagerPriv
{

public:

    PixmapManagerPriv()
    {
        size     = 0;
        cache    = 0;
        view     = 0;
        thumbJob = 0;
    }

    int                        size;

    QCache<KUrl, QPixmap>     *cache;
    QPointer<ThumbnailJob>     thumbJob;
    QString                    thumbCacheDir;

    AlbumIconView             *view;
};

PixmapManager::PixmapManager(AlbumIconView* view)
{
    d = new PixmapManagerPriv;
    d->view  = view;
    d->cache = new QCache<KUrl, QPixmap>(100);
    d->thumbCacheDir = QDir::homePath() + "/.thumbnails/";
}

PixmapManager::~PixmapManager()
{
    if (d->thumbJob)
    {
        d->thumbJob->kill();
    }

    delete d->cache;
    delete d;
}

void PixmapManager::setThumbnailSize(int size)
{
    if (d->size == size)
        return;

    d->size = size;
    d->cache->clear();
    if (d->thumbJob)
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }
}

QPixmap PixmapManager::find(const KUrl& url)
{
    QPixmap* pix = d->cache->object(url);
    if (pix)
        return *pix;

    if (!d->thumbJob)
    {
        d->thumbJob = new ThumbnailJob(url, d->size, true, AlbumSettings::instance()->getExifRotate());

        connect(d->thumbJob, SIGNAL(signalThumbnail(const KUrl&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KUrl&, const QPixmap&)));

        connect(d->thumbJob, SIGNAL(signalFailed(const KUrl&)),
                this, SLOT(slotFailedThumbnail(const KUrl&)));

        connect(d->thumbJob, SIGNAL(signalCompleted()),
                this, SLOT(slotCompleted()));
    }

    return QPixmap();
}

void PixmapManager::deleteThumbnail(const KUrl& url)
{
    d->cache->remove(url);

    if (!d->thumbJob.isNull())
        d->thumbJob->removeItem(url);

    // remove the items from the thumbnail cache directory as well.
    QString uri = "file://" + QDir::cleanPath(url.path());
    KMD5 md5(QFile::encodeName(uri));
    uri = md5.hexDigest();

    QString smallThumbPath = d->thumbCacheDir + "normal/" + uri + ".png";
    QString bigThumbPath   = d->thumbCacheDir + "large/"  + uri + ".png";

    ::unlink(QFile::encodeName(smallThumbPath));
    ::unlink(QFile::encodeName(bigThumbPath));
}

void PixmapManager::clear()
{
    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }

    d->cache->clear();
}

void PixmapManager::slotGotThumbnail(const KUrl& url, const QPixmap& pix)
{
    d->cache->remove(url);
    QPixmap* thumb = new QPixmap(pix);
    d->cache->insert(url, thumb);
    emit signalPixmap(url);
}

void PixmapManager::slotFailedThumbnail(const KUrl& url)
{
    QPixmap pix;
    QString ext = QFileInfo(url.path()).suffix();

    // Wrapper around mime type of item to get the right icon.

    AlbumSettings* settings = AlbumSettings::instance();
    if (settings)
    {
        if (settings->getImageFileFilter().toUpper().contains(ext.toUpper()) ||
            settings->getRawFileFilter().toUpper().contains(ext.toUpper()))
        {
            pix = DesktopIcon("image", KIconLoader::SizeEnormous);
        }
        else if (settings->getMovieFileFilter().toUpper().contains(ext.toUpper()))
        {
            pix = DesktopIcon("video", KIconLoader::SizeEnormous);
        }
        else if (settings->getAudioFileFilter().toUpper().contains(ext.toUpper()))
        {
            pix = DesktopIcon("sound", KIconLoader::SizeEnormous);
        }
    }

    if (pix.isNull())
        pix = DesktopIcon("image-missing", KIconLoader::SizeEnormous);

    // Resize icon to the right size depending of current settings.

    QSize size(pix.size());
    size.scale(d->size, d->size, Qt::KeepAspectRatio);
    if (size.width() < pix.width() && size.height() < pix.height())
    {
        // only scale down
        // do not scale up, looks bad
        pix = pix.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    d->cache->remove(url);
    d->cache->insert(url, new QPixmap(pix));
    emit signalPixmap(url);
}

void PixmapManager::slotCompleted()
{
    d->thumbJob->kill();
    d->thumbJob = 0;

    AlbumIconItem* item = d->view->nextItemToThumbnail();
    if (!item)
        return;

    find(item->imageInfo().fileUrl());
}

int PixmapManager::cacheSize() const
{
    return d->cache->maxCost();
}

}  // namespace Digikam
