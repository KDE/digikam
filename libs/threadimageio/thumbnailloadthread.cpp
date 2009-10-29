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

#include "databaseparameters.h"
#include "iccmanager.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "thumbnaildatabaseaccess.h"
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

class ThumbnailLoadThreadStaticPriv
{
public:

    ThumbnailLoadThreadStaticPriv()
    {
        storageMethod      = ThumbnailCreator::FreeDesktopStandard;
        provider           = 0;
        displayingWidget   = 0;
        firstThreadCreated = false;
    }

    ~ThumbnailLoadThreadStaticPriv()
    {
        delete provider;
    }

    ThumbnailCreator::StorageMethod  storageMethod;
    ThumbnailInfoProvider           *provider;
    QWidget                         *displayingWidget;

    bool firstThreadCreated;
};

K_GLOBAL_STATIC(ThumbnailLoadThreadStaticPriv, static_d)

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

    QHash<QString, ThumbnailResult> collectedResults;
    QMutex                          resultsMutex;

    QList<LoadingDescription>       kdeTodo;
    QHash<KUrl, LoadingDescription> kdeJobHash;
    KIO::PreviewJob                *kdeJob;

    LoadingDescription createLoadingDescription(const QString filePath, int size);
};

K_GLOBAL_STATIC(ThumbnailLoadThread, defaultIconViewObject)
K_GLOBAL_STATIC(ThumbnailLoadThread, defaultObject)
K_GLOBAL_STATIC(ThumbnailLoadThread, defaultThumbBarObject)

ThumbnailLoadThread::ThumbnailLoadThread()
                   : d(new ThumbnailLoadThreadPriv)
{
    static_d->firstThreadCreated = true;
    d->creator = new ThumbnailCreator(static_d->storageMethod);
    if (static_d->provider)
        d->creator->setThumbnailInfoProvider(static_d->provider);
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

void ThumbnailLoadThread::initializeThumbnailDatabase(const QString &thumbnailDBFile, ThumbnailInfoProvider *provider)
{
    if (static_d->firstThreadCreated)
    {
        kError() << "Call initializeThumbnailDatabase at application start. "
                    "There are already thumbnail loading threads created, "
                    "and these will not be switched to use the database. ";
    }
    ThumbnailDatabaseAccess::setParameters(DatabaseParameters::parametersForSQLite(thumbnailDBFile));
    if (ThumbnailDatabaseAccess::checkReadyForUse(0))
    {
        kDebug() << "Thumbnail db ready for use";
        static_d->storageMethod = ThumbnailCreator::ThumbnailDatabase;
        static_d->provider = provider;
    }
    else
    {
        kError() << "Failed to initialize thumbnail database at" << thumbnailDBFile
                                << "\n Error message:" << ThumbnailDatabaseAccess().lastError();
    }
}

void ThumbnailLoadThread::setDisplayingWidget(QWidget *widget)
{
    static_d->displayingWidget = widget;
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

bool ThumbnailLoadThread::find(const QString& filePath, QPixmap& retPixmap)
{
    return find(filePath, retPixmap, d->size);
}

LoadingDescription ThumbnailLoadThreadPriv::createLoadingDescription(const QString filePath, int size)
{
    // bug #206666: Do not cut off one-pixel line for highlighting border
    if (highlight && size >= 10)
        size -= 2;

    LoadingDescription description(filePath, size, exifRotate,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);

    if (IccSettings::instance()->isEnabled())
    {
        description.postProcessingParameters.colorManagement = LoadingDescription::ConvertForDisplay;
        description.postProcessingParameters.setProfile(IccManager::displayProfile(static_d->displayingWidget));
    }

    return description;
}

bool ThumbnailLoadThread::find(const QString& filePath, QPixmap& retPixmap, int size)
{
    const QPixmap *pix;
    LoadingDescription description = d->createLoadingDescription(filePath, size);
    QString cacheKey = description.cacheKey();

    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        pix = cache->retrieveThumbnailPixmap(cacheKey);
    }

    if (pix)
    {
        retPixmap = QPixmap(*pix);
        return true;
    }

    {
        // If there is a result waiting for conversion to pixmap, return false - pixmap will come shortly
        QMutexLocker lock(&d->resultsMutex);
        if (d->collectedResults.contains(cacheKey))
            return false;
    }

    load(description);
    return false;
}

void ThumbnailLoadThread::find(const QString& filePath)
{
    find(filePath, d->size);
}

void ThumbnailLoadThread::find(const QString& filePath, int size)
{
    const QPixmap *pix;
    LoadingDescription description = d->createLoadingDescription(filePath, size);
    QString cacheKey = description.cacheKey();

    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        pix = cache->retrieveThumbnailPixmap(cacheKey);
    }

    if (pix)
    {
        emit signalThumbnailLoaded(description, QPixmap(*pix));
        return;
    }

    {
        // If there is a result waiting for conversion to pixmap, return false - pixmap will come shortly
        QMutexLocker lock(&d->resultsMutex);
        if (d->collectedResults.contains(cacheKey))
            return;
    }

    load(description);
}

void ThumbnailLoadThread::findGroup(const QStringList& filePaths)
{
    findGroup(filePaths, d->size);
}

void ThumbnailLoadThread::findGroup(const QStringList& filePaths, int size)
{
    if (!checkSize(size))
        return;

    QList<LoadingDescription> descriptions;
    {
        LoadingCache *cache = LoadingCache::cache();
        foreach(const QString& filePath, filePaths)
        {
            LoadingDescription description = d->createLoadingDescription(filePath, size);
            QString cacheKey = description.cacheKey();

            {
                LoadingCache::CacheLock lock(cache);
                if (cache->retrieveThumbnailPixmap(cacheKey))
                    continue;
            }

            {
                QMutexLocker lock(&d->resultsMutex);
                if (d->collectedResults.contains(cacheKey))
                    continue;
            }

            descriptions << description;
        }
    }
    ManagedLoadSaveThread::prependThumbnailGroup(descriptions);
}

void ThumbnailLoadThread::preload(const QString& filePath)
{
    preload(filePath, d->size);
}

void ThumbnailLoadThread::preload(const QString& filePath, int size)
{
    LoadingDescription description = d->createLoadingDescription(filePath, size);
    QString cacheKey = description.cacheKey();

    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        if (cache->retrieveThumbnailPixmap(cacheKey))
            return;
    }

    {
        QMutexLocker lock(&d->resultsMutex);
        if (d->collectedResults.contains(cacheKey))
            return;
    }

    load(description, true);
}

void ThumbnailLoadThread::load(const LoadingDescription& desc)
{
    load(desc, false);
}

void ThumbnailLoadThread::load(const LoadingDescription& constDescription, bool preload)
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
        kError() << "ThumbnailLoadThread::load: No thumbnail size specified. Refusing to load thumbnail.";
        return false;
    }
    else if (size > ThumbnailSize::Huge)
    {
        kError() << "ThumbnailLoadThread::load: Thumbnail size " << size
                      << " is larger than " << ThumbnailSize::Huge << ". Refusing to load.";
        return false;
    }
    return true;
}

// virtual method overridden from LoadSaveNotifier, implemented first by LoadSaveThread
// called by ThumbnailTask from working thread
void ThumbnailLoadThread::thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img)
{
    // call parent to send signalThumbnailLoaded(LoadingDescription, QImage) - signal is part of public API
    ManagedLoadSaveThread::thumbnailLoaded(loadingDescription, img);

    // Store result in our list and fire one signal
    // This means there can be several results per pixmap,
    // to speed up cases where inter-thread communication is the limiting factor
    QMutexLocker lock(&d->resultsMutex);
    d->collectedResults.insert(loadingDescription.cacheKey(), ThumbnailResult(loadingDescription, img));
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
        results = d->collectedResults.values();
        d->collectedResults.clear();
        // reset flag so that for next result, the signal is sent again
        d->notifiedForResults = false;
    }

    foreach(const ThumbnailResult& result, results)
        slotThumbnailLoaded(result.loadingDescription, result.image);
}

void ThumbnailLoadThread::slotThumbnailLoaded(const LoadingDescription& description, const QImage& thumb)
{
    if (thumb.isNull())
        loadWithKDE(description);

    QPixmap pix;

    int w = thumb.width();
    int h = thumb.height();

    // highlight only when requested and when thumbnail
    // width and height are greater than 10
    if (d->highlight && (w >= 10 && h >= 10))
    {
        pix = QPixmap(w + 2, h + 2);
        QPainter p(&pix);
        p.setPen(QPen(Qt::black, 1));
        p.drawRect(0, 0, w + 1, h + 1);
        p.drawImage(1, 1, thumb);
    }
    else
    {
        pix = QPixmap::fromImage(thumb);
    }

    // put into cache
    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->putThumbnail(description.cacheKey(), pix, description.filePath);
    }

    emit signalThumbnailLoaded(description, pix);
}

void ThumbnailLoadThread::loadWithKDE(const LoadingDescription& description)
{
    d->kdeTodo << description;
    startKdePreviewJob();
}

void ThumbnailLoadThread::startKdePreviewJob()
{
    if (d->kdeJob || d->kdeTodo.isEmpty())
        return;

    KUrl::List list;
    foreach (const LoadingDescription& description, d->kdeTodo)
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

void ThumbnailLoadThread::gotKDEPreview(const KFileItem& item, const QPixmap& kdepix)
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

void ThumbnailLoadThread::failedKDEPreview(const KFileItem& item)
{
    gotKDEPreview(item, QPixmap());
}

void ThumbnailLoadThread::kdePreviewFinished(KJob *)
{
    d->kdeJob = 0;
    startKdePreviewJob();
}

QPixmap ThumbnailLoadThread::surrogatePixmap(const LoadingDescription& description)
{
    QPixmap pix;

    KMimeType::Ptr mimeType = KMimeType::findByPath(description.filePath);
    if (mimeType)
    {
        pix = DesktopIcon(mimeType->iconName(), KIconLoader::SizeEnormous);
    }

    /*
    No dependency on AlbumSettings here please...
    QString ext = QFileInfo(url.toLocalFile()).suffix();

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

void ThumbnailLoadThread::deleteThumbnail(const QString& filePath)
{
    {
        LoadingCache *cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        QStringList possibleKeys = LoadingDescription::possibleThumbnailCacheKeys(filePath);
        foreach(const QString& cacheKey, possibleKeys)
            cache->removeThumbnail(cacheKey);
    }

    ThumbnailCreator creator(static_d->storageMethod);
    if (static_d->provider)
        creator.setThumbnailInfoProvider(static_d->provider);
    creator.deleteThumbnailsFromDisk(filePath);
}

}   // namespace Digikam
