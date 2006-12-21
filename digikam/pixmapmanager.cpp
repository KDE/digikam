/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2005-04-14
 * Description : a pixmap manager for album icon view.  
 * 
 * Copyright 2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

#include <qcache.h>
#include <qguardedptr.h>
#include <qpixmap.h>
#include <qdir.h>
#include <qfile.h>
#include <qtimer.h>
#include <qimage.h>

// KDE includes.

#include <kmdcodec.h>
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
        timer    = 0;
        thumbJob = 0;
    }

    int                        size;

    QCache<QPixmap>           *cache;
    QGuardedPtr<ThumbnailJob>  thumbJob;
    QTimer                    *timer;
    QString                    thumbCacheDir;

    AlbumIconView             *view;
};

PixmapManager::PixmapManager(AlbumIconView* view)
{
    d = new PixmapManagerPriv;
    d->view  = view;
    d->cache = new QCache<QPixmap>(101, 211);
    d->cache->setAutoDelete(true);
    d->thumbCacheDir = QDir::homeDirPath() + "/.thumbnails/";
    
    d->timer = new QTimer();
    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotCompleted()));
}

PixmapManager::~PixmapManager()
{
    delete d->timer;
    
    if (!d->thumbJob.isNull())
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
    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }
}

QPixmap* PixmapManager::find(const KURL& url)
{
    QPixmap* pix = d->cache->find(url.path());
    if (pix)
        return pix;
    
    if (d->thumbJob.isNull())
    {
        d->thumbJob = new ThumbnailJob(url, d->size, true, AlbumSettings::instance()->getExifRotate());
        
        connect(d->thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));

        connect(d->thumbJob, SIGNAL(signalFailed(const KURL&)),
                this, SLOT(slotFailedThumbnail(const KURL&)));

        connect(d->thumbJob, SIGNAL(signalCompleted()),
                this, SLOT(slotCompleted()));
    }
    
    return 0;
}

void PixmapManager::remove(const KURL& url)
{
    d->cache->remove(url.path());

    if (!d->thumbJob.isNull())
        d->thumbJob->removeItem(url);

    // remove the items from the thumbnail cache directory as well.
    QString uri = "file://" + QDir::cleanDirPath(url.path());
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

void PixmapManager::slotGotThumbnail(const KURL& url, const QPixmap& pix)
{
    d->cache->remove(url.path());
    QPixmap* thumb = new QPixmap(pix);
    d->cache->insert(url.path(), thumb);
    emit signalPixmap(url);
}

void PixmapManager::slotFailedThumbnail(const KURL& url)
{
    QImage img;
    QString ext = QFileInfo(url.path()).extension(false);

    // Wrapper around mime type of item to get the right icon.

    AlbumSettings* settings = AlbumSettings::instance();
    if (settings)
    {
        if (settings->getImageFileFilter().upper().contains(ext.upper()) ||
            settings->getRawFileFilter().upper().contains(ext.upper()))
        { 
            img = DesktopIcon("image", KIcon::SizeEnormous).convertToImage();
        }
        else if (settings->getMovieFileFilter().upper().contains(ext.upper()))
        {
            img = DesktopIcon("video", KIcon::SizeEnormous).convertToImage();
        }
        else if (settings->getAudioFileFilter().upper().contains(ext.upper()))
        {
            img = DesktopIcon("sound", KIcon::SizeEnormous).convertToImage();
        }
    }

    if (img.isNull())
        img = DesktopIcon("file_broken", KIcon::SizeEnormous).convertToImage();

    // Resize icon to the right size depending of current settings.

    QSize size(img.size());
    size.scale(d->size, d->size, QSize::ScaleMin);
    if (size.width() < img.width() && size.height() < img.height())
    {
        // only scale down
        // do not scale up, looks bad
        img = img.smoothScale(size);
    }

    d->cache->remove(url.path());
    QPixmap* thumb = new QPixmap(img);
    d->cache->insert(url.path(), thumb);
    emit signalPixmap(url);
}

void PixmapManager::slotCompleted()
{
    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }

    AlbumIconItem* item = d->view->nextItemToThumbnail();
    if (!item)
        return;

    find(item->imageInfo()->kurl());
}

int PixmapManager::cacheSize() const
{
    return d->cache->maxCost();    
}

}  // namespace Digikam
