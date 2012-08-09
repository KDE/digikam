/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2005-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbnailloadthread.moc"

// Qt includes

#include <QEventLoop>
#include <QHash>
#include <QPainter>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kio/previewjob.h>
#include <kmessagebox.h>
#include <kdeversion.h>

// Local includes

#include "databaseparameters.h"
#include "iccmanager.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "metadatasettings.h"
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

    ThumbnailCreator::StorageMethod storageMethod;
    ThumbnailInfoProvider*          provider;
    QWidget*                        displayingWidget;

    bool                            firstThreadCreated;
};

K_GLOBAL_STATIC(ThumbnailLoadThreadStaticPriv, static_d)

// -------------------------------------------------------------------

class ThumbnailLoadThread::ThumbnailLoadThreadPriv
{

public:

    ThumbnailLoadThreadPriv()
    {
        size               = ThumbnailSize::Huge;
        wantPixmap         = true;
        highlight          = true;
        sendSurrogate      = true;
        creator            = 0;
        kdeJob             = 0;
        notifiedForResults = false;
    }

    bool                            wantPixmap;
    bool                            highlight;
    bool                            sendSurrogate;
    bool                            notifiedForResults;

    int                             size;

    ThumbnailCreator*               creator;

    QHash<QString, ThumbnailResult> collectedResults;
    QMutex                          resultsMutex;

    QList<LoadingDescription>       kdeTodo;
    QHash<KUrl, LoadingDescription> kdeJobHash;
    KIO::PreviewJob*                kdeJob;

    QList<LoadingDescription>       lastDescriptions;
    QStringList                     previewPlugins;
public:

    LoadingDescription        createLoadingDescription(const QString& filePath, int size, bool setLastDescription = true);
    LoadingDescription        createLoadingDescription(const QString& filePath, int size,
                                                       const QRect& detailRect, bool setLastDescription = true);
    bool                      checkDescription(const LoadingDescription& description);
    QList<LoadingDescription> makeDescriptions(const QStringList& filePaths, int size);
    QList<LoadingDescription> makeDescriptions(const QList<QPair<QString, QRect> >& filePathsAndRects, int size);
    bool                      hasHighlightingBorder() const;
    int                       pixmapSizeForThumbnailSize(int thumbnailSize) const;
    int                       thumbnailSizeForPixmapSize(int pixmapSize) const;
};

K_GLOBAL_STATIC(ThumbnailLoadThread, defaultIconViewObject)
K_GLOBAL_STATIC(ThumbnailLoadThread, defaultObject)
K_GLOBAL_STATIC(ThumbnailLoadThread, defaultThumbBarObject)

ThumbnailLoadThread::ThumbnailLoadThread(QObject* parent)
    : ManagedLoadSaveThread(parent),
      d(new ThumbnailLoadThreadPriv)
{
    static_d->firstThreadCreated = true;
    d->creator                   = new ThumbnailCreator(static_d->storageMethod);

    if (static_d->provider)
    {
        d->creator->setThumbnailInfoProvider(static_d->provider);
    }

    d->creator->setOnlyLargeThumbnails(true);
    d->creator->setRemoveAlphaChannel(true);

    connect(this, SIGNAL(thumbnailsAvailable()),
            this, SLOT(slotThumbnailsAvailable()));
}

ThumbnailLoadThread::~ThumbnailLoadThread()
{
    shutDown();
    delete d->creator;
    delete d;
}

ThumbnailLoadThread* ThumbnailLoadThread::defaultIconViewThread()
{
    return defaultIconViewObject;
}

ThumbnailLoadThread* ThumbnailLoadThread::defaultThread()
{
    return defaultObject;
}

ThumbnailLoadThread* ThumbnailLoadThread::defaultThumbBarThread()
{
    return defaultThumbBarObject;
}

void ThumbnailLoadThread::cleanUp()
{
    defaultIconViewObject.destroy();
    defaultObject.destroy();
    defaultThumbBarObject.destroy();
}

void ThumbnailLoadThread::initializeThumbnailDatabase(const DatabaseParameters& params, ThumbnailInfoProvider* provider)
{
    if (static_d->firstThreadCreated)
    {
        kError() << "Call initializeThumbnailDatabase at application start. "
                 "There are already thumbnail loading threads created, "
                 "and these will not be switched to use the database. ";
    }

    ThumbnailDatabaseAccess::setParameters(params);

    if (ThumbnailDatabaseAccess::checkReadyForUse(0))
    {
        kDebug() << "Thumbnail db ready for use";
        static_d->storageMethod = ThumbnailCreator::ThumbnailDatabase;
        static_d->provider      = provider;
    }
    else
    {
        KMessageBox::information(0, i18n("Error message: %1").arg(ThumbnailDatabaseAccess().lastError()),
                                 i18n("Failed to initialize thumbnail database"));
    }
}

void ThumbnailLoadThread::setDisplayingWidget(QWidget* widget)
{
    static_d->displayingWidget = widget;
}

void ThumbnailLoadThread::setThumbnailSize(int size, bool forFace)
{
    d->size = size;

    if (forFace)
    {
        d->creator->setThumbnailSize(size);
    }
}

int ThumbnailLoadThread::maximumThumbnailSize()
{
    return ThumbnailSize::Huge;
}

int ThumbnailLoadThread::maximumThumbnailPixmapSize(bool highlight)
{
    if (highlight)
    {
        return ThumbnailSize::Huge;
    }
    else
    {
        return ThumbnailSize::Huge + 2;    // see slotThumbnailLoaded
    }
}

void ThumbnailLoadThread::setSendSurrogatePixmap(bool send)
{
    d->sendSurrogate = send;
}

void ThumbnailLoadThread::setPixmapRequested(bool wantPixmap)
{
    d->wantPixmap = wantPixmap;
}

void ThumbnailLoadThread::setHighlightPixmap(bool highlight)
{
    d->highlight = highlight;
}

ThumbnailCreator* ThumbnailLoadThread::thumbnailCreator() const
{
    return d->creator;
}

int ThumbnailLoadThread::thumbnailPixmapSize(int size) const
{
    return d->pixmapSizeForThumbnailSize(size);
}

int ThumbnailLoadThread::thumbnailPixmapSize(bool withHighlight, int size)
{
    if (withHighlight && size >= 10)
    {
        return size + 2;
    }

    return size;
}

bool ThumbnailLoadThread::ThumbnailLoadThreadPriv::hasHighlightingBorder() const
{
    return highlight && size >= 10;
}

int ThumbnailLoadThread::ThumbnailLoadThreadPriv::pixmapSizeForThumbnailSize(int thumbnailSize) const
{
    if (hasHighlightingBorder())
    {
        return thumbnailSize + 2;
    }

    return thumbnailSize;
}

int ThumbnailLoadThread::ThumbnailLoadThreadPriv::thumbnailSizeForPixmapSize(int pixmapSize) const
{
    // bug #206666: Do not cut off one-pixel line for highlighting border
    if (hasHighlightingBorder())
    {
        return pixmapSize - 2;
    }

    return pixmapSize;
}

// --- Creating loading descriptions ---

LoadingDescription ThumbnailLoadThread::ThumbnailLoadThreadPriv
     ::createLoadingDescription(const QString& filePath, int size, bool setLastDescription)
{
    size = thumbnailSizeForPixmapSize(size);

    LoadingDescription description(filePath, size,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);

    if (IccSettings::instance()->isEnabled())
    {
        description.postProcessingParameters.colorManagement = LoadingDescription::ConvertForDisplay;
        description.postProcessingParameters.setProfile(IccManager::displayProfile(static_d->displayingWidget));
    }

    if (setLastDescription)
    {
        lastDescriptions.clear();
        lastDescriptions << description;
    }

    return description;
}

LoadingDescription ThumbnailLoadThread::ThumbnailLoadThreadPriv
    ::createLoadingDescription(const QString& filePath, int size, const QRect& detailRect, bool setLastDescription)
{
    size = thumbnailSizeForPixmapSize(size);

    LoadingDescription description(filePath, size,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::DetailThumbnail);

    description.previewParameters.extraParameter = detailRect;

    if (IccSettings::instance()->isEnabled())
    {
        description.postProcessingParameters.colorManagement = LoadingDescription::ConvertForDisplay;
        description.postProcessingParameters.setProfile(IccManager::displayProfile(static_d->displayingWidget));
    }

    if (setLastDescription)
    {
        lastDescriptions.clear();
        lastDescriptions << description;
    }

    return description;
}

bool ThumbnailLoadThread::ThumbnailLoadThreadPriv::checkDescription(const LoadingDescription& description)
{
    QString cacheKey = description.cacheKey();

    {
        LoadingCache* cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        if (cache->hasThumbnailPixmap(cacheKey))
        {
            return false;
        }
    }

    {
        QMutexLocker lock(&resultsMutex);

        if (collectedResults.contains(cacheKey))
        {
            return false;
        }
    }
    return true;
}

QList<LoadingDescription> ThumbnailLoadThread::ThumbnailLoadThreadPriv::makeDescriptions(const QStringList& filePaths, int size)
{
    QList<LoadingDescription> descriptions;
    {
        LoadingDescription description = createLoadingDescription(QString(), size, false);
        foreach(const QString& filePath, filePaths)
        {
            description.filePath = filePath;

            if (!checkDescription(description))
            {
                continue;
            }

            descriptions << description;
        }
    }
    lastDescriptions = descriptions;
    return descriptions;
}

QList<LoadingDescription> ThumbnailLoadThread::ThumbnailLoadThreadPriv::makeDescriptions(const QList<QPair<QString, QRect> >& filePathsAndRects, int size)
{
    QList<LoadingDescription> descriptions;
    {
        LoadingDescription description = createLoadingDescription(QString(), size, QRect(1,1,1,1), false);
        typedef QPair<QString, QRect> StringRectPair;
        foreach(const StringRectPair& pair, filePathsAndRects)
        {
            description.filePath = pair.first;

            if (!checkDescription(description))
            {
                continue;
            }

            description.previewParameters.extraParameter = pair.second;
            descriptions << description;
        }
    }
    lastDescriptions = descriptions;
    return descriptions;
}

bool ThumbnailLoadThread::find(const QString& filePath, int size, QPixmap* retPixmap, bool emitSignal, const QRect& detailRect)
{
    const QPixmap* pix = 0;
    LoadingDescription description;

    if (detailRect.isNull())
    {
        description = d->createLoadingDescription(filePath, size);
    }
    else
    {
        description = d->createLoadingDescription(filePath, size, detailRect);
    }

    QString cacheKey = description.cacheKey();

    {
        LoadingCache* cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        pix                 = cache->retrieveThumbnailPixmap(cacheKey);
    }

    if (pix)
    {
        if (retPixmap)
        {
            *retPixmap = *pix;
        }

        if (emitSignal)
        {
            emit signalThumbnailLoaded(description, QPixmap(*pix));
        }

        return true;
    }

    {
        // If there is a result waiting for conversion to pixmap, return false - pixmap will come shortly
        QMutexLocker lock(&d->resultsMutex);

        if (d->collectedResults.contains(cacheKey))
        {
            return false;
        }
    }

    load(description);
    return false;
}

// --- Normal thumbnails ---

bool ThumbnailLoadThread::find(const QString& filePath, QPixmap& retPixmap, int size)
{
    return find(filePath, size, &retPixmap, false, QRect());
}

bool ThumbnailLoadThread::find(const QString& filePath, QPixmap& retPixmap)
{
    return find(filePath, retPixmap, d->size);
}

void ThumbnailLoadThread::find(const QString& filePath)
{
    find(filePath, d->size);
}

void ThumbnailLoadThread::find(const QString& filePath, int size)
{
    find(filePath, size, 0, true, QRect());
}

void ThumbnailLoadThread::findGroup(const QStringList& filePaths)
{
    findGroup(filePaths, d->size);
}

void ThumbnailLoadThread::findGroup(const QStringList& filePaths, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(filePaths, size);
    ManagedLoadSaveThread::prependThumbnailGroup(descriptions);
}

// --- Detail thumbnails ---

bool ThumbnailLoadThread::find(const QString& filePath, const QRect& rect, QPixmap& pixmap)
{
    return find(filePath, rect, pixmap, d->size);
}

bool ThumbnailLoadThread::find(const QString& filePath, const QRect& rect, QPixmap& pixmap, int size)
{
    return find(filePath, size, &pixmap, false, rect);
}

void ThumbnailLoadThread::find(const QString& filePath, const QRect& rect)
{
    find(filePath, rect, d->size);
}

void ThumbnailLoadThread::find(const QString& filePath, const QRect& rect, int size)
{
    find(filePath, size, 0, true, rect);
}

void ThumbnailLoadThread::findGroup(const QList<QPair<QString, QRect> >& filePathAndRects)
{
    findGroup(filePathAndRects, d->size);
}

void ThumbnailLoadThread::findGroup(const QList<QPair<QString, QRect> >& filePathsAndRects, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(filePathsAndRects, size);
    ManagedLoadSaveThread::prependThumbnailGroup(descriptions);
}

// --- Preloading ---

void ThumbnailLoadThread::preload(const QString& filePath)
{
    preload(filePath, d->size);
}

void ThumbnailLoadThread::preload(const QString& filePath, int size)
{
    LoadingDescription description = d->createLoadingDescription(filePath, size);

    if (d->checkDescription(description))
    {
        load(description, true);
    }
}

void ThumbnailLoadThread::preloadGroup(const QStringList& filePaths)
{
    pregenerateGroup(filePaths, d->size);
}

void ThumbnailLoadThread::preloadGroup(const QStringList& filePaths, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(filePaths, size);
    ManagedLoadSaveThread::preloadThumbnailGroup(descriptions);
}

void ThumbnailLoadThread::pregenerateGroup(const QStringList& filePaths)
{
    pregenerateGroup(filePaths, d->size);
}

void ThumbnailLoadThread::pregenerateGroup(const QStringList& filePaths, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(filePaths, size);

    for (int i=0; i<descriptions.size(); ++i)
    {
        descriptions[i].previewParameters.flags |= LoadingDescription::PreviewParameters::OnlyPregenerate;
    }

    ManagedLoadSaveThread::preloadThumbnailGroup(descriptions);
}

// --- Basic load() ---

void ThumbnailLoadThread::load(const LoadingDescription& desc)
{
    load(desc, false);
}

void ThumbnailLoadThread::load(const LoadingDescription& description, bool preload)
{
    if (!checkSize(description.previewParameters.size))
    {
        return;
    }

    if (preload)
    {
        ManagedLoadSaveThread::preloadThumbnail(description);
    }
    else
    {
        ManagedLoadSaveThread::loadThumbnail(description);
    }
}

QList<LoadingDescription> ThumbnailLoadThread::lastDescriptions() const
{
    return d->lastDescriptions;
}

bool ThumbnailLoadThread::checkSize(int size)
{
    size = d->thumbnailSizeForPixmapSize(size);

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

// --- Receiving ---

// virtual method overridden from LoadSaveNotifier, implemented first by LoadSaveThread
// called by ThumbnailTask from working thread
void ThumbnailLoadThread::thumbnailLoaded(const LoadingDescription& loadingDescription, const QImage& img)
{
    // call parent to send signalThumbnailLoaded(LoadingDescription, QImage) - signal is part of public API
    ManagedLoadSaveThread::thumbnailLoaded(loadingDescription, img);

    if (!d->wantPixmap)
    {
        return;
    }

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
        results               = d->collectedResults.values();
        d->collectedResults.clear();
        // reset flag so that for next result, the signal is sent again
        d->notifiedForResults = false;
    }

    foreach(const ThumbnailResult& result, results)
    {
        slotThumbnailLoaded(result.loadingDescription, result.image);
    }
}

void ThumbnailLoadThread::slotThumbnailLoaded(const LoadingDescription& description, const QImage& thumb)
{
    if (thumb.isNull())
    {
        loadWithKDE(description);
    }

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
        LoadingCache* cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->putThumbnail(description.cacheKey(), pix, description.filePath);
    }

    emit signalThumbnailLoaded(description, pix);
}

// --- KDE thumbnails ---

void ThumbnailLoadThread::loadWithKDE(const LoadingDescription& description)
{
    d->kdeTodo << description;
    startKdePreviewJob();
}

void ThumbnailLoadThread::startKdePreviewJob()
{
    if (d->kdeJob || d->kdeTodo.isEmpty())
    {
        return;
    }

    d->kdeJobHash.clear();
    KUrl::List list;
    foreach(const LoadingDescription& description, d->kdeTodo)
    {
        KUrl url = KUrl::fromPath(description.filePath);
        list << url;
        d->kdeJobHash[url] = description;
    }
    d->kdeTodo.clear();

#if KDE_IS_VERSION(4,7,0)
    KFileItemList items;
    if (d->previewPlugins.isEmpty())
      d->previewPlugins = KIO::PreviewJob::availablePlugins();
    
    for (KUrl::List::ConstIterator it = list.begin() ; it != list.end() ; ++it)
    {
        if ((*it).isValid())
            items.append(KFileItem(KFileItem::Unknown, KFileItem::Unknown, *it, true));
    }
    d->kdeJob = KIO::filePreview(items, QSize(d->creator->storedSize(), d->creator->storedSize()), &d->previewPlugins); // FIXME: do not know if size 0 is allowed
#else
    d->kdeJob = KIO::filePreview(list, d->creator->storedSize());                                   // FIXME: do not know if size 0 is allowed
#endif

    connect(d->kdeJob, SIGNAL(gotPreview(KFileItem,QPixmap)),
            this, SLOT(gotKDEPreview(KFileItem,QPixmap)));

    connect(d->kdeJob, SIGNAL(failed(KFileItem)),
            this, SLOT(failedKDEPreview(KFileItem)));

    connect(d->kdeJob, SIGNAL(finished(KJob*)),
            this, SLOT(kdePreviewFinished(KJob*)));
}

void ThumbnailLoadThread::gotKDEPreview(const KFileItem& item, const QPixmap& kdepix)
{
    if (!d->kdeJobHash.contains(item.url()))
    {
        return;
    }

    LoadingDescription description = d->kdeJobHash.value(item.url());
    QPixmap pix;

    if (kdepix.isNull())
    {
        // third and last attempt - load a mimetype specific icon
        pix = surrogatePixmap(description);
    }
    else
    {
        d->creator->store(description.filePath, kdepix.toImage());
        pix = kdepix.scaled(description.previewParameters.size, description.previewParameters.size,
                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // put into cache
    {
        LoadingCache* cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->putThumbnail(description.cacheKey(), pix, description.filePath);
    }

    emit signalThumbnailLoaded(description, pix);
}

void ThumbnailLoadThread::failedKDEPreview(const KFileItem& item)
{
    gotKDEPreview(item, QPixmap());
}

void ThumbnailLoadThread::kdePreviewFinished(KJob*)
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
    {
        pix = DesktopIcon("image-missing", KIconLoader::SizeEnormous);
    }

    if (pix.isNull())
    {
        // give up
        return QPixmap();
    }

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

// --- Utilities ---

void ThumbnailLoadThread::storeDetailThumbnail(const QString& filePath, const QRect& detailRect, const QImage& image, bool isFace)
{
    d->creator->storeDetailThumbnail(filePath, detailRect, image, isFace);
}

int ThumbnailLoadThread::storedSize() const
{
    return d->creator->storedSize();
}

void ThumbnailLoadThread::deleteThumbnail(const QString& filePath)
{
    {
        LoadingCache* cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);

        QStringList possibleKeys = LoadingDescription::possibleThumbnailCacheKeys(filePath);
        foreach(const QString& cacheKey, possibleKeys)
        {
            cache->removeThumbnail(cacheKey);
        }
    }

    ThumbnailCreator creator(static_d->storageMethod);

    if (static_d->provider)
    {
        creator.setThumbnailInfoProvider(static_d->provider);
    }

    creator.deleteThumbnailsFromDisk(filePath);
}

// --- ThumbnailImageCatcher

class ThumbnailImageCatcherResult
{
public:

    ThumbnailImageCatcherResult(const LoadingDescription& d)
        : description(d), received(false)
    {
    }

    ThumbnailImageCatcherResult(const LoadingDescription& d, const QImage& image)
        : image(image), description(d), received(true)
    {
    }

    QImage             image;
    LoadingDescription description;
    bool               received;
};

enum ThumbnailImageCatcherState
{
    Inactive,
    Accepting,
    Waiting,
    Quitting
};

class ThumbnailImageCatcher::ThumbnailImageCatcherPriv
{
public:

    ThumbnailImageCatcherPriv()
    {
        state   = Inactive;
        thread  = 0;
        active  = true;
    }

    ThumbnailImageCatcherState         state;

    bool                               active;
    ThumbnailLoadThread*               thread;
    QList<ThumbnailImageCatcherResult> tasks;
    QList<ThumbnailImageCatcherResult> intermediate;

    QMutex                             mutex;
    QWaitCondition                     condVar;

    void reset();
    void harvest(const LoadingDescription& description, const QImage& image);
};

ThumbnailImageCatcher::ThumbnailImageCatcher(QObject* parent)
    : QObject(parent), d(new ThumbnailImageCatcherPriv)
{
}

ThumbnailImageCatcher::ThumbnailImageCatcher(ThumbnailLoadThread* thread, QObject* parent)
    : QObject(parent), d(new ThumbnailImageCatcherPriv)
{
    setThumbnailLoadThread(thread);
}

ThumbnailImageCatcher::~ThumbnailImageCatcher()
{
    delete d;
}

ThumbnailLoadThread* ThumbnailImageCatcher::thread() const
{
    return d->thread;
}

void ThumbnailImageCatcher::setThumbnailLoadThread(ThumbnailLoadThread* thread)
{
    if (d->thread == thread)
    {
        return;
    }

    d->state = Inactive;

    if (d->thread)
    {
        disconnect(thread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QImage)),
                   this, SLOT(slotThumbnailLoaded(LoadingDescription,QImage)));
    }

    d->thread = thread;

    {
        QMutexLocker(&d->mutex);
        d->reset();
    }

    if (d->thread)
    {
        connect(thread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QImage)),
                this, SLOT(slotThumbnailLoaded(LoadingDescription,QImage)),
                Qt::DirectConnection
               );
    }

}

void ThumbnailImageCatcher::setActive(bool active)
{
    if (d->active == active)
    {
        return;
    }

    if (!active)
    {
        cancel();
    }

    QMutexLocker lock(&d->mutex);
    d->active = active;
    d->reset();
}

void ThumbnailImageCatcher::cancel()
{
    QMutexLocker lock(&d->mutex);

    if (d->state == Waiting)
    {
        d->state = Quitting;
        d->condVar.wakeOne();
    }
}

void ThumbnailImageCatcher::ThumbnailImageCatcherPriv::reset()
{
    intermediate.clear();
    tasks.clear();

    if (active)
    {
        state = Accepting;
    }
    else
    {
        state = Inactive;
    }
}

void ThumbnailImageCatcher::ThumbnailImageCatcherPriv::harvest(const LoadingDescription& description, const QImage& image)
{
    // called under lock
    bool finished = true;

    for (int i=0; i<tasks.size(); ++i)
    {
        ThumbnailImageCatcherResult& task = tasks[i];

        if (task.description == description)
        {
            task.image    = image;
            task.received = true;
        }

        finished = finished && task.received;
    }

    if (finished)
    {
        state = Quitting;
        condVar.wakeOne();
    }
}

void ThumbnailImageCatcher::slotThumbnailLoaded(const LoadingDescription& description, const QImage& image)
{
    // We are in the thumbnail thread here, DirectConnection!

    QMutexLocker(&d->mutex);

    switch (d->state)
    {
        case Inactive:
            break;
        case Accepting:
            d->intermediate << ThumbnailImageCatcherResult(description, image);
            break;
        case Waiting:
            d->harvest(description, image);
            break;
        case Quitting:
            break;
    }
}

int ThumbnailImageCatcher::enqueue()
{
    QList<LoadingDescription> descriptions = d->thread->lastDescriptions();

    QMutexLocker(&d->mutex);
    foreach(const LoadingDescription& description, descriptions)
    {
        d->tasks << description;
    }

    return descriptions.size();
}

QList<QImage> ThumbnailImageCatcher::waitForThumbnails()
{
    if (!d->thread || d->tasks.isEmpty() || !d->active)
    {
        return QList<QImage>();
    }

    QMutexLocker lock(&d->mutex);
    d->state = Waiting;

    // first, handle results received between request and calling this method
    foreach(const ThumbnailImageCatcherResult& result, d->intermediate)
    {
        d->harvest(result.description, result.image);
    }
    d->intermediate.clear();

    // Now wait for the rest to arrive. If already finished, state will be Quitting
    while (d->state == Waiting)
    {
        d->condVar.wait(&d->mutex);
    }

    QList<QImage> result;
    foreach(const ThumbnailImageCatcherResult& task, d->tasks)
    {
        result << task.image;
    }

    d->reset();

    return result;
}

}   // namespace Digikam
