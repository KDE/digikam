/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbnailloadthread.h"
#include "thumbnailloadthread.moc"

// Qt includes

#include <QPainter>
#include <QHash>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/previewjob.h>

// Local includes

#include "thumbnailsize.h"
#include "thumbnailtask.h"
#include "thumbnailcreator.h"

namespace Digikam
{

class ThumbnailResult
{

public:

    ThumbnailResult(LoadingDescription description, QImage image)
        : loadingDescription(description), image(image)
    {
    }

    LoadingDescription loadingDescription;
    QImage             image;
};

// -------------------------------------------------------------------

class ThumbnailLoadThreadPriv
{

public:

    ThumbnailLoadThreadPriv()
    {
        size               = ThumbnailSize::Huge;
        exifRotate         = true;
        highlight          = true;
        sendSurrogate      = true;
        creator            = 0;
        kdeJob             = 0;
        notifiedForResults = false;
    }

    bool                            exifRotate;
    bool                            highlight;
    bool                            sendSurrogate;
    bool                            notifiedForResults;

    int                             size;

    ThumbnailCreator*               creator;

    QList<ThumbnailResult>          collectedResults;
    QMutex                          resultsMutex;

    QList<LoadingDescription>       kdeTodo;
    QHash<KUrl, LoadingDescription> kdeJobHash;
    KIO::PreviewJob                *kdeJob;
};

K_GLOBAL_STATIC(ThumbnailLoadThread, defaultIconViewObject)
K_GLOBAL_STATIC(ThumbnailLoadThread, defaultObject)
K_GLOBAL_STATIC(ThumbnailLoadThread, defaultThumbBarObject)

ThumbnailLoadThread::ThumbnailLoadThread()
                   : d(new ThumbnailLoadThreadPriv)
{
    d->creator = new ThumbnailCreator();
    d->creator->setOnlyLargeThumbnails(true);
    d->creator->setRemoveAlphaChannel(true);
    setPixmapRequested(true);
}

ThumbnailLoadThread::~ThumbnailLoadThread()
{
    shutdownThread();
    delete d->creator;
    delete d;
}

ThumbnailLoadThread *ThumbnailLoadThread::defaultIconViewThread()
{
    return defaultIconViewObject;
}

ThumbnailLoadThread *ThumbnailLoadThread::defaultThread()
{
    return defaultObject;
}

ThumbnailLoadThread *ThumbnailLoadThread::defaultThumbBarThread()
{
    return defaultThumbBarObject;
}

void ThumbnailLoadThread::cleanUp()
{
    defaultIconViewObject.destroy();
    defaultObject.destroy();
    defaultThumbBarObject.destroy();
}

void ThumbnailLoadThread::setThumbnailSize(int size)
{
    d->size = size;
}

int ThumbnailLoadThread::maximumThumbnailSize()
{
    return ThumbnailSize::Huge;
}

void ThumbnailLoadThread::setExifRotate(int exifRotate)
{
    d->exifRotate = exifRotate;
}

bool ThumbnailLoadThread::exifRotate() const
{
    return d->exifRotate;
}

void ThumbnailLoadThread::setSendSurrogatePixmap(bool send)
{
    d->sendSurrogate = send;
}

void ThumbnailLoadThread::setPixmapRequested(bool wantPixmap)
{
    if (wantPixmap)
        connect(this, SIGNAL(thumbnailsAvailable()),
                this, SLOT(slotThumbnailsAvailable()));
    else
        disconnect(this, SIGNAL(thumbnailsAvailable()),
                   this, SLOT(slotThumbnailsAvailable()));
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
    return find(filePath, retPixmap, d->size);
}

bool ThumbnailLoadThread::find(const QString &filePath, QPixmap &retPixmap, int size)
{
    const QPixmap *pix;
    LoadingDescription description(filePath, size, d->exifRotate, LoadingDescription::PreviewParameters::Thumbnail);

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
    find(filePath, d->size);
}

void ThumbnailLoadThread::find(const QString &filePath, int size)
{
    const QPixmap *pix;
    LoadingDescription description(filePath, size, d->exifRotate, LoadingDescription::PreviewParameters::Thumbnail);

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

void ThumbnailLoadThread::findGroup(const QStringList &filePaths)
{
    findGroup(filePaths, d->size);
}

void ThumbnailLoadThread::findGroup(const QStringList &filePaths, int size)
{
    if (!checkSize(size))
        return;

    QList<LoadingDescription> descriptions;
    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        foreach(const QString filePath, filePaths)
        {
            LoadingDescription description(filePath, size, d->exifRotate, LoadingDescription::PreviewParameters::Thumbnail);
            if (!cache->retrieveThumbnailPixmap(description.cacheKey()))
                descriptions << description;
        }
    }
    ManagedLoadSaveThread::prependThumbnailGroup(descriptions);
}

void ThumbnailLoadThread::preload(const QString &filePath)
{
    preload(filePath, d->size);
}

void ThumbnailLoadThread::preload(const QString &filePath, int size)
{
    LoadingDescription description(filePath, size, d->exifRotate, LoadingDescription::PreviewParameters::Thumbnail);

    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        if (cache->retrieveThumbnailPixmap(description.cacheKey()))
            return;
    }

    load(description, true);
}

void ThumbnailLoadThread::load(const LoadingDescription &desc)
{
    load(desc, false);
}

void ThumbnailLoadThread::load(const LoadingDescription &constDescription, bool preload)
{
    LoadingDescription description(constDescription);

    if (!checkSize(description.previewParameters.size))
        return;

    if (preload)
        ManagedLoadSaveThread::preloadThumbnail(description);
    else
        ManagedLoadSaveThread::loadThumbnail(description);
}

bool ThumbnailLoadThread::checkSize(int size)
{
    if (size <= 0)
    {
        kError(50003) << "ThumbnailLoadThread::load: No thumbnail size specified. Refusing to load thumbnail."
                      << endl;
        return false;
    }
    else if (size > ThumbnailSize::Huge)
    {
        kError(50003) << "ThumbnailLoadThread::load: Thumbnail size " << size
                      << " is larger than " << ThumbnailSize::Huge << ". Refusing to load." << endl;
        return false;
    }
    return true;
}

// virtual method overridden from LoadSaveNotifier, implemented first by LoadSaveThread
// called by ThumbnailTask from working thread
void ThumbnailLoadThread::thumbnailLoaded(const LoadingDescription &loadingDescription, const QImage& img)
{
    // call parent to send signalThumbnailLoaded(LoadingDescription, QImage) - signal is part of public API
    ManagedLoadSaveThread::thumbnailLoaded(loadingDescription, img);

    // Store result in our list and fire one signal
    // This means there can be several results per pixmap,
    // to speed up cases where inter-thread communication is the limiting factor
    QMutexLocker lock(&d->resultsMutex);
    d->collectedResults << ThumbnailResult(loadingDescription, img);
    // only sent signal when flag indicates there is no signal on the way currently
    if (!d->notifiedForResults)
    {
        d->notifiedForResults = true;
        emit thumbnailsAvailable();
    }
}

void ThumbnailLoadThread::slotThumbnailsAvailable()
{
    // harvest collected results safely into our thread
    QList<ThumbnailResult> results;
    {
        QMutexLocker lock(&d->resultsMutex);
        results = d->collectedResults;
        d->collectedResults.clear();
        // reset flag so that for next result, the signal is sent again
        d->notifiedForResults = false;
    }

    foreach(const ThumbnailResult &result, results)
        slotThumbnailLoaded(result.loadingDescription, result.image);
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
        cache->putThumbnail(description.cacheKey(), pix, description.filePath);
    }

    emit signalThumbnailLoaded(description, pix);
}

void ThumbnailLoadThread::loadWithKDE(const LoadingDescription &description)
{
    d->kdeTodo << description;
    startKdePreviewJob();
}

void ThumbnailLoadThread::startKdePreviewJob()
{
    if (d->kdeJob || d->kdeTodo.isEmpty())
        return;

    KUrl::List list;
    foreach (const LoadingDescription description, d->kdeTodo)
    {
        KUrl url = KUrl::fromPath(description.filePath);
        list << url;
        d->kdeJobHash[url] = description;
    }
    d->kdeTodo.clear();
    d->kdeJob = KIO::filePreview(list, d->size);

    connect(d->kdeJob, SIGNAL(gotPreview(const KFileItem &, const QPixmap &)),
            this, SLOT(gotKDEPreview(const KFileItem &, const QPixmap &)));

    connect(d->kdeJob, SIGNAL(failed(const KFileItem &)),
            this, SLOT(failedKDEPreview(const KFileItem &)));

    connect(d->kdeJob, SIGNAL(finished(KJob*)),
            this, SLOT(kdePreviewFinished(KJob*)));
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
        cache->putThumbnail(description.cacheKey(), pix, description.filePath);
    }

    emit signalThumbnailLoaded(description, pix);
}

void ThumbnailLoadThread::failedKDEPreview(const KFileItem &item)
{
    gotKDEPreview(item, QPixmap());
}

void ThumbnailLoadThread::kdePreviewFinished(KJob *)
{
    d->kdeJob = 0;
    startKdePreviewJob();    
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
        pix = DesktopIcon("image-missing", KIconLoader::SizeEnormous);

    if (pix.isNull())
        // give up
        return QPixmap();

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
        foreach(const QString &cacheKey, possibleKeys)
            cache->removeThumbnail(cacheKey);
    }

    ThumbnailCreator::deleteThumbnailsFromDisk(filePath);
}

}   // namespace Digikam
