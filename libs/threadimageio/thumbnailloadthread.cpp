/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QPainter>
#include <QHash>

// KDE includes

#include <kglobal.h>
#include <kiconloader.h>
#include <kio/previewjob.h>

// Local includes.

#include "ddebug.h"
#include "thumbnailtask.h"
#include "thumbnailcreator.h"
#include "thumbnailloadthread.h"
#include "thumbnailloadthread.moc"

namespace Digikam
{

class ThumbnailLoadThreadPriv
{
public:

    ThumbnailLoadThreadPriv()
    {
        size          = 128;
        exifRotate    = true;
        highlight     = true;
        sendSurrogate = true;
        creator       = 0;
    }

    ThumbnailCreator *creator;
    int  size;
    bool exifRotate;
    bool highlight;
    bool sendSurrogate;
    QHash<KUrl, LoadingDescription> kdeJobHash;
};

K_GLOBAL_STATIC(ThumbnailLoadThread, defaultObject);

ThumbnailLoadThread::ThumbnailLoadThread()
{
    d = new ThumbnailLoadThreadPriv;

    d->creator = new ThumbnailCreator();
    //d->creator->setOnlyLargeThumbnails(true);
    d->creator->setRemoveAlphaChannel(true);
    setPixmapRequested(true);
}

ThumbnailLoadThread::~ThumbnailLoadThread()
{
    delete d;
}

ThumbnailLoadThread *ThumbnailLoadThread::defaultThread()
{
    return defaultObject;
}

void ThumbnailLoadThread::setThumbnailSize(int size)
{
    d->size = size;
}

void ThumbnailLoadThread::setExifRotate(int exifRotate)
{
    d->exifRotate = exifRotate;
}

void ThumbnailLoadThread::setSendSurrogatePixmap(bool send)
{
    d->sendSurrogate = send;
}

void ThumbnailLoadThread::setPixmapRequested(bool wantPixmap)
{
    if (wantPixmap)
        connect(this, SIGNAL(signalThumbnailLoaded(const LoadingDescription &, const QImage&)),
                this, SLOT(slotThumbnailLoaded(const LoadingDescription &, const QImage&)));
    else
        disconnect(this, SIGNAL(signalThumbnailLoaded(const LoadingDescription &, const QImage&)),
                   this, SLOT(slotThumbnailLoaded(const LoadingDescription &, const QImage&)));
}

void ThumbnailLoadThread::setHighlightPixmap(bool highlight)
{
    d->highlight = highlight;
}

ThumbnailCreator *ThumbnailLoadThread::thumbnailCreator() const
{
    return d->creator;
}

bool ThumbnailLoadThread::find(const QString &filePath, QPixmap &retPixmap)
{
    const QPixmap *pix;
    LoadingDescription description(filePath, d->size, d->exifRotate, LoadingDescription::PreviewParameters::Thumbnail);

    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        pix = cache->retrieveThumbnailPixmap(description.cacheKey());
    }

    if (pix)
    {
        retPixmap = QPixmap(*pix);
        return true;
    }

    load(description);
    return false;
}

void ThumbnailLoadThread::find(const QString &filePath)
{
    const QPixmap *pix;
    LoadingDescription description(filePath, d->size, d->exifRotate, LoadingDescription::PreviewParameters::Thumbnail);

    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        pix = cache->retrieveThumbnailPixmap(description.cacheKey());
    }

    if (pix)
    {
        emit signalThumbnailLoaded(description, QPixmap(*pix));
        return;
    }

    load(description);
}

void ThumbnailLoadThread::load(const LoadingDescription &constDescription)
{
    LoadingDescription description(constDescription);

    if (description.previewParameters.size <= 0)
    {
        DError() << "ThumbnailLoadThread::load: No thumbnail size specified. Refusing to load thumbnail." << endl;
        return;
    }
    else if (description.previewParameters.size > 256)
    {
        DError() << "ThumbnailLoadThread::load: Thumbnail size " << description.previewParameters.size
                 << " is larger than 256. Refusing to load." << endl;
        return;
    }

    ManagedLoadSaveThread::loadThumbnail(description);
}

void ThumbnailLoadThread::slotThumbnailLoaded(const LoadingDescription &description, const QImage& thumb)
{
    if (thumb.isNull())
        loadWithKDE(description);

    QPixmap pix = QPixmap::fromImage(thumb);

    int w = pix.width();
    int h = pix.height();

    // highlight only when requested and when thumbnail
    // width and height are greater than 10
    if (d->highlight && (w >= 10 && h >= 10))
    {
        QPainter p(&pix);
        p.setPen(QPen(Qt::black, 1));
        p.drawRect(0, 0, w - 1, h - 1);
    }

    // put into cache
    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->putThumbnail(description.cacheKey(), pix);
    }

    emit signalThumbnailLoaded(description, pix);
}

void ThumbnailLoadThread::loadWithKDE(const LoadingDescription &description)
{
    // try again with KDE preview
    KUrl url = KUrl::fromPath(description.filePath);
    KUrl::List list;
    list << url;
    KIO::PreviewJob *job = KIO::filePreview(list, d->size);
    d->kdeJobHash[url] = description;

    connect(job, SIGNAL(gotPreview(const KFileItem &, const QPixmap &)),
            this, SLOT(gotKDEPreview(const KFileItem &, const QPixmap &)));
    connect(job, SIGNAL(failed(const KFileItem &)),
            this, SLOT(failedKDEPreview(const KFileItem &)));
}

void ThumbnailLoadThread::gotKDEPreview(const KFileItem &item, const QPixmap &kdepix)
{
    QPixmap pix(kdepix);
    LoadingDescription description = d->kdeJobHash[item.url()];

    // third and last attempt - load a mimetype specific icon
    if (pix.isNull() && d->sendSurrogate)
        pix = surrogatePixmap(description);

    // put into cache
    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->putThumbnail(description.cacheKey(), pix);
    }

    emit signalThumbnailLoaded(description, pix);
}

void ThumbnailLoadThread::failedKDEPreview(const KFileItem &item)
{
    gotKDEPreview(item, QPixmap());
}

QPixmap ThumbnailLoadThread::surrogatePixmap(const LoadingDescription &description)
{
    QPixmap pix;

    KMimeType::Ptr mimeType = KMimeType::findByPath(description.filePath);
    if (mimeType)
    {
        pix = DesktopIcon(mimeType->iconName(), KIconLoader::SizeEnormous);
    }

    /*
    No dependency on AlbumSettings here please...
    QString ext = QFileInfo(url.path()).suffix();

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
    */

    if (pix.isNull())
        pix = DesktopIcon("file-broken", KIconLoader::SizeEnormous);

    // Resize icon to the right size depending of current settings.

    QSize size(pix.size());
    size.scale(description.previewParameters.size, description.previewParameters.size, Qt::KeepAspectRatio);
    if (!pix.isNull() && size.width() < pix.width() && size.height() < pix.height())
    {
        // only scale down
        // do not scale up, looks bad
        pix = pix.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    return pix;
}

void ThumbnailLoadThread::deleteThumbnail(const QString &filePath)
{
    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        QStringList possibleKeys = LoadingDescription::possibleThumbnailCacheKeys(filePath);
        foreach(QString cacheKey, possibleKeys)
            cache->removeThumbnail(cacheKey);
    }

    ThumbnailCreator::deleteThumbnailsFromDisk(filePath);
}



}   // namespace Digikam
