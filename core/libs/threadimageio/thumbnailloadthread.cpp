/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

// Qt includes

#include <QApplication>
#include <QEventLoop>
#include <QHash>
#include <QPainter>
#include <QMessageBox>
#include <QIcon>
#include <QMimeType>
#include <QMimeDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dbengineparameters.h"
#include "iccmanager.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "metadatasettings.h"
#include "thumbsdbaccess.h"
#include "thumbnailsize.h"
#include "thumbnailtask.h"
#include "thumbnailcreator.h"

#ifdef HAVE_MEDIAPLAYER
#   include "videothumbnailerjob.h"
#endif

namespace Digikam
{

class ThumbnailResult
{

public:

    ThumbnailResult(const LoadingDescription& description, const QImage& image)
        : loadingDescription(description),
          image(image)
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
      : firstThreadCreated(false),
        storageMethod(ThumbnailCreator::FreeDesktopStandard),
        provider(0),
        profile(IccProfile::sRGB())
    {
    }

    ~ThumbnailLoadThreadStaticPriv()
    {
        delete provider;
    }

public:

    bool                            firstThreadCreated;

    ThumbnailCreator::StorageMethod storageMethod;
    ThumbnailInfoProvider*          provider;
    IccProfile                      profile;
};

Q_GLOBAL_STATIC(ThumbnailLoadThreadStaticPriv, static_d)

// -------------------------------------------------------------------

class ThumbnailLoadThread::Private
{

public:

    Private()
    {
        size               = ThumbnailSize::maxThumbsSize();
        wantPixmap         = true;
        highlight          = true;
        sendSurrogate      = true;
        notifiedForResults = false;
        creator            = 0;

#ifdef HAVE_MEDIAPLAYER
        videoThumbs        = 0;
#endif
    }

    bool                               wantPixmap;
    bool                               highlight;
    bool                               sendSurrogate;
    bool                               notifiedForResults;

    int                                size;

    ThumbnailCreator*                  creator;

    QHash<QString, ThumbnailResult>    collectedResults;
    QMutex                             resultsMutex;

    QHash<QString, LoadingDescription> videoJobHash;

#ifdef HAVE_MEDIAPLAYER
    VideoThumbnailerJob*               videoThumbs;
#endif

    QList<LoadingDescription>          lastDescriptions;

public:

    LoadingDescription        createLoadingDescription(const ThumbnailIdentifier& identifier, int size, bool setLastDescription = true);
    LoadingDescription        createLoadingDescription(const ThumbnailIdentifier& identifier, int size,
                                                       const QRect& detailRect, bool setLastDescription = true);
    bool                      checkDescription(const LoadingDescription& description);
    QList<LoadingDescription> makeDescriptions(const QList<ThumbnailIdentifier>& identifiers, int size);
    QList<LoadingDescription> makeDescriptions(const QList<QPair<ThumbnailIdentifier, QRect> >& idsAndRects, int size);
    bool                      hasHighlightingBorder() const;
    int                       pixmapSizeForThumbnailSize(int thumbnailSize) const;
    int                       thumbnailSizeForPixmapSize(int pixmapSize) const;
};

Q_GLOBAL_STATIC(ThumbnailLoadThread, defaultIconViewObject)
Q_GLOBAL_STATIC(ThumbnailLoadThread, defaultObject)
Q_GLOBAL_STATIC(ThumbnailLoadThread, defaultThumbBarObject)

ThumbnailLoadThread::ThumbnailLoadThread(QObject* const parent)
    : ManagedLoadSaveThread(parent),
      d(new Private)
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

#ifdef HAVE_MEDIAPLAYER

    d->videoThumbs               = new VideoThumbnailerJob(this);
    d->videoThumbs->setCreateStrip(true);

    connect(d->videoThumbs, SIGNAL(signalThumbnailDone(QString,QImage)),
            this, SLOT(slotVideoThumbnailDone(QString,QImage)));

    connect(d->videoThumbs, SIGNAL(signalThumbnailFailed(QString)),
            this, SLOT(slotVideoThumbnailFailed(QString)));

    connect(d->videoThumbs, SIGNAL(signalThumbnailJobFinished()),
            this, SLOT(slotVideoThumbnailFinished()));

#endif
}

ThumbnailLoadThread::~ThumbnailLoadThread()
{
    shutDown();

#ifdef HAVE_MEDIAPLAYER
    delete d->videoThumbs;
#endif

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
    // NOTE : Nothing to do with Qt5 and Q_GLOBAL_STATIC. Qt clean up all automatically at end of application instance.
    // But stopping all running tasks to prevent a crash at end.
    defaultIconViewThread()->stopAllTasks();
    defaultThumbBarThread()->stopAllTasks();
    defaultThread()->stopAllTasks();
}

void ThumbnailLoadThread::initializeThumbnailDatabase(const DbEngineParameters& params, ThumbnailInfoProvider* const provider)
{
    if (static_d->firstThreadCreated)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Call initializeThumbnailDatabase at application start. "
                                        "There are already thumbnail loading threads created, "
                                        "and these will not be switched to use the database. ";
    }

    ThumbsDbAccess::setParameters(params);

    if (ThumbsDbAccess::checkReadyForUse(0))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Thumbnails database ready for use";
        static_d->storageMethod = ThumbnailCreator::ThumbnailDatabase;
        static_d->provider      = provider;
    }
    else
    {
        QMessageBox::information(qApp->activeWindow(), i18n("Failed to initialize thumbnails database"),
                                 i18n("Error message: %1", ThumbsDbAccess().lastError()));
    }
}

void ThumbnailLoadThread::setDisplayingWidget(QWidget* const widget)
{
    static_d->profile = IccManager::displayProfile(widget);
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
    return ThumbnailSize::maxThumbsSize();
}

int ThumbnailLoadThread::maximumThumbnailPixmapSize(bool highlight)
{
    if (highlight)
    {
        return ThumbnailSize::maxThumbsSize();
    }
    else
    {
        return ThumbnailSize::maxThumbsSize() + 2;    // see slotThumbnailLoaded
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

int ThumbnailLoadThread::thumbnailToPixmapSize(int size) const
{
    return d->pixmapSizeForThumbnailSize(size);
}

int ThumbnailLoadThread::thumbnailToPixmapSize(bool withHighlight, int size)
{
    if (withHighlight && size >= 10)
    {
        return size + 2;
    }

    return size;
}

int ThumbnailLoadThread::pixmapToThumbnailSize(int size) const
{
    return d->thumbnailSizeForPixmapSize(size);
}

bool ThumbnailLoadThread::Private::hasHighlightingBorder() const
{
    return highlight && size >= 10;
}

int ThumbnailLoadThread::Private::pixmapSizeForThumbnailSize(int thumbnailSize) const
{
    if (hasHighlightingBorder())
    {
        return thumbnailSize + 2;
    }

    return thumbnailSize;
}

int ThumbnailLoadThread::Private::thumbnailSizeForPixmapSize(int pixmapSize) const
{
    // bug #206666: Do not cut off one-pixel line for highlighting border
    if (hasHighlightingBorder())
    {
        return pixmapSize - 2;
    }

    return pixmapSize;
}

// --- Creating loading descriptions ---

LoadingDescription ThumbnailLoadThread::Private::createLoadingDescription(const ThumbnailIdentifier& identifier, int size,
                                                                          bool setLastDescription)
{
    size = thumbnailSizeForPixmapSize(size);

    LoadingDescription description(identifier.filePath, PreviewSettings(), size,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::Thumbnail);
    description.previewParameters.storageReference = identifier.id;

    if (IccSettings::instance()->useManagedPreviews())
    {
        description.postProcessingParameters.colorManagement = LoadingDescription::ConvertForDisplay;
        description.postProcessingParameters.setProfile(static_d->profile);
    }

    if (setLastDescription)
    {
        lastDescriptions.clear();
        lastDescriptions << description;
    }

    return description;
}

LoadingDescription ThumbnailLoadThread::Private::createLoadingDescription(const ThumbnailIdentifier& identifier, int size,
                                                                          const QRect& detailRect, bool setLastDescription)
{
    size = thumbnailSizeForPixmapSize(size);

    LoadingDescription description(identifier.filePath, PreviewSettings(), size,
                                   LoadingDescription::NoColorConversion,
                                   LoadingDescription::PreviewParameters::DetailThumbnail);
    description.previewParameters.storageReference = identifier.id;

    description.previewParameters.extraParameter = detailRect;

    if (IccSettings::instance()->useManagedPreviews())
    {
        description.postProcessingParameters.colorManagement = LoadingDescription::ConvertForDisplay;
        description.postProcessingParameters.setProfile(static_d->profile);
    }

    if (setLastDescription)
    {
        lastDescriptions.clear();
        lastDescriptions << description;
    }

    return description;
}

bool ThumbnailLoadThread::Private::checkDescription(const LoadingDescription& description)
{
    QString cacheKey = description.cacheKey();

    {
        LoadingCache* const cache = LoadingCache::cache();
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

QList<LoadingDescription> ThumbnailLoadThread::Private::makeDescriptions(const QList<ThumbnailIdentifier>& identifiers, int size)
{
    QList<LoadingDescription> descriptions;
    {
        LoadingDescription description = createLoadingDescription(ThumbnailIdentifier(), size, false);

        foreach(const ThumbnailIdentifier& identifier, identifiers)
        {
            description.filePath = identifier.filePath;
            description.previewParameters.storageReference = identifier.id;

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

QList<LoadingDescription> ThumbnailLoadThread::Private::makeDescriptions(const QList<QPair<ThumbnailIdentifier, QRect> >& identifiersAndRects, int size)
{
    QList<LoadingDescription> descriptions;
    {
        LoadingDescription description = createLoadingDescription(ThumbnailIdentifier(), size, QRect(1,1,1,1), false);
        typedef QPair<ThumbnailIdentifier, QRect> IdRectPair;

        foreach(const IdRectPair& pair, identifiersAndRects)
        {
            description.filePath = pair.first.filePath;
            description.previewParameters.storageReference = pair.first.id;

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

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, int size, QPixmap* retPixmap, bool emitSignal, const QRect& detailRect)
{
    const QPixmap* pix = 0;
    LoadingDescription description;

    if (detailRect.isNull())
    {
        description = d->createLoadingDescription(identifier, size);
    }
    else
    {
        description = d->createLoadingDescription(identifier, size, detailRect);
    }

    QString cacheKey = description.cacheKey();

    {
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        pix                       = cache->retrieveThumbnailPixmap(cacheKey);
    }

    if (pix)
    {
        if (retPixmap)
        {
            *retPixmap = *pix;
        }

        if (emitSignal)
        {
            load(description);
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

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, QPixmap& retPixmap, int size)
{
    return find(identifier, size, &retPixmap, false, QRect());
}

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, QPixmap& retPixmap)
{
    return find(identifier, retPixmap, d->size);
}

void ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier)
{
    find(identifier, d->size);
}

void ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, int size)
{
    find(identifier, size, 0, true, QRect());
}

void ThumbnailLoadThread::findGroup(QList<ThumbnailIdentifier>& identifiers)
{
    findGroup(identifiers, d->size);
}

void ThumbnailLoadThread::findGroup(QList<ThumbnailIdentifier>& identifiers, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(identifiers, size);
    ManagedLoadSaveThread::prependThumbnailGroup(descriptions);
}

// --- Detail thumbnails ---

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, const QRect& rect, QPixmap& pixmap)
{
    return find(identifier, rect, pixmap, d->size);
}

bool ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, const QRect& rect, QPixmap& pixmap, int size)
{
    return find(identifier, size, &pixmap, false, rect);
}

void ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, const QRect& rect)
{
    find(identifier, rect, d->size);
}

void ThumbnailLoadThread::find(const ThumbnailIdentifier& identifier, const QRect& rect, int size)
{
    find(identifier, size, 0, true, rect);
}

void ThumbnailLoadThread::findGroup(const QList<QPair<ThumbnailIdentifier, QRect> >& idsAndRects)
{
    findGroup(idsAndRects, d->size);
}

void ThumbnailLoadThread::findGroup(const QList<QPair<ThumbnailIdentifier, QRect> >& idsAndRects, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(idsAndRects, size);
    ManagedLoadSaveThread::prependThumbnailGroup(descriptions);
}

// --- Preloading ---

void ThumbnailLoadThread::preload(const ThumbnailIdentifier& identifier)
{
    preload(identifier, d->size);
}

void ThumbnailLoadThread::preload(const ThumbnailIdentifier& identifier, int size)
{
    LoadingDescription description = d->createLoadingDescription(identifier, size);

    if (d->checkDescription(description))
    {
        load(description, true);
    }
}

void ThumbnailLoadThread::preloadGroup(QList<ThumbnailIdentifier>& identifiers)
{
    preloadGroup(identifiers, d->size);
}

void ThumbnailLoadThread::preloadGroup(QList<ThumbnailIdentifier>& identifiers, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(identifiers, size);
    ManagedLoadSaveThread::preloadThumbnailGroup(descriptions);
}

void ThumbnailLoadThread::pregenerateGroup(const QList<ThumbnailIdentifier>& identifiers)
{
    pregenerateGroup(identifiers, d->size);
}

void ThumbnailLoadThread::pregenerateGroup(const QList<ThumbnailIdentifier>& identifiers, int size)
{
    if (!checkSize(size))
    {
        return;
    }

    QList<LoadingDescription> descriptions = d->makeDescriptions(identifiers, size);

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
        qCDebug(DIGIKAM_GENERAL_LOG) << "ThumbnailLoadThread::load: No thumbnail size specified. Refusing to load thumbnail.";
        return false;
    }
    else if (size > ThumbnailSize::maxThumbsSize())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "ThumbnailLoadThread::load: Thumbnail size " << size
                                     << " is larger than " << ThumbnailSize::maxThumbsSize() << ". Refusing to load.";
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
    QPixmap pix;

    if (thumb.isNull())
    {
        loadVideoThumbnail(description);
        pix = surrogatePixmap(description);
    }
    else
    {
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
    }

    // put into cache
    {
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->putThumbnail(description.cacheKey(), pix, description.filePath);
    }

    emit signalThumbnailLoaded(description, pix);
}

// --- Video thumbnails ---

void ThumbnailLoadThread::loadVideoThumbnail(const LoadingDescription& description)
{
#ifdef HAVE_MEDIAPLAYER
    d->videoJobHash.insert(description.filePath, description);
    d->videoThumbs->setThumbnailSize(d->creator->storedSize());
    d->videoThumbs->setExifRotate(MetadataSettings::instance()->settings().exifRotate);
    d->videoThumbs->addItems(QStringList() << description.filePath);
#else
    qDebug(DIGIKAM_GENERAL_LOG) << "Cannot get video thumb for " << description.filePath;
    qDebug(DIGIKAM_GENERAL_LOG) << "Video support is not available";
#endif
}

void ThumbnailLoadThread::slotVideoThumbnailDone(const QString& item, const QImage& img)
{
    if (!d->videoJobHash.contains(item))
    {
        return;
    }

    LoadingDescription description = d->videoJobHash.value(item);
    QPixmap pix;

    if (img.isNull())
    {
        // third and last attempt - load a mimetype specific icon
        pix = surrogatePixmap(description);
    }
    else
    {
        d->creator->store(description.filePath, img);
        pix = QPixmap::fromImage(img.scaled(description.previewParameters.size, description.previewParameters.size,
                                 Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // put into cache
    {
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        cache->putThumbnail(description.cacheKey(), pix, description.filePath);
    }

    d->videoJobHash.remove(description.filePath);

    emit signalThumbnailLoaded(description, pix);
}

void ThumbnailLoadThread::slotVideoThumbnailFailed(const QString& item)
{
    slotVideoThumbnailDone(item, QImage());
}

void ThumbnailLoadThread::slotVideoThumbnailFinished()
{
}

QPixmap ThumbnailLoadThread::surrogatePixmap(const LoadingDescription& description)
{
    QPixmap pix;

    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(description.filePath);

    if (mimeType.isValid())
    {
        pix = QIcon::fromTheme(mimeType.genericIconName()).pixmap(128);
    }

    if (pix.isNull())
    {
        pix = QIcon::fromTheme(QLatin1String("application-x-zerosize")).pixmap(128);
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
    Q_UNUSED(isFace);
    d->creator->storeDetailThumbnail(filePath, detailRect, image);
}

int ThumbnailLoadThread::storedSize() const
{
    return d->creator->storedSize();
}

void ThumbnailLoadThread::deleteThumbnail(const QString& filePath)
{
    {
        LoadingCache* const cache = LoadingCache::cache();
        LoadingCache::CacheLock lock(cache);
        QStringList possibleKeys  = LoadingDescription::possibleThumbnailCacheKeys(filePath);

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

// --- ThumbnailImageCatcher ---------------------------------------------------------

class ThumbnailImageCatcher::Private
{

public:

    enum CatcherState
    {
        Inactive,
        Accepting,
        Waiting,
        Quitting
    };

public:

    class CatcherResult
    {
    public:

        CatcherResult(const LoadingDescription& d)
            : description(d),
              received(false)
        {
        }

        CatcherResult(const LoadingDescription& d, const QImage& image)
            : image(image),
              description(d),
              received(true)
        {
        }

    public:

        QImage             image;
        LoadingDescription description;
        bool               received;
    };

public:

    Private()
    {
        state   = Inactive;
        thread  = 0;
        active  = true;
    }

    void reset();
    void harvest(const LoadingDescription& description, const QImage& image);

public:

    CatcherState                  state;

    bool                          active;
    ThumbnailLoadThread*          thread;
    QList<Private::CatcherResult> tasks;
    QList<Private::CatcherResult> intermediate;

    QMutex                        mutex;
    QWaitCondition                condVar;
};

void ThumbnailImageCatcher::Private::reset()
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

void ThumbnailImageCatcher::Private::harvest(const LoadingDescription& description, const QImage& image)
{
    // called under lock
    bool finished = true;

    for (int i = 0 ; i < tasks.size() ; ++i)
    {
        Private::CatcherResult& task = tasks[i];

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

ThumbnailImageCatcher::ThumbnailImageCatcher(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
}

ThumbnailImageCatcher::ThumbnailImageCatcher(ThumbnailLoadThread* const thread, QObject* const parent)
    : QObject(parent),
      d(new Private)
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

void ThumbnailImageCatcher::setThumbnailLoadThread(ThumbnailLoadThread* const thread)
{
    if (d->thread == thread)
    {
        return;
    }

    d->state = Private::Inactive;

    if (d->thread)
    {
        disconnect(d->thread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QImage)),
                   this, SLOT(slotThumbnailLoaded(LoadingDescription,QImage)));
    }

    d->thread = thread;

    {
        QMutexLocker lock(&d->mutex);
        d->reset();
    }

    if (d->thread)
    {
        connect(d->thread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QImage)),
                this, SLOT(slotThumbnailLoaded(LoadingDescription,QImage)),
                Qt::DirectConnection);
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

    if (d->state == Private::Waiting)
    {
        d->state = Private::Quitting;
        d->condVar.wakeOne();
    }
}

void ThumbnailImageCatcher::slotThumbnailLoaded(const LoadingDescription& description, const QImage& image)
{
    // We are in the thumbnail thread here, DirectConnection!

    QMutexLocker lock(&d->mutex);

    switch (d->state)
    {
        case Private::Inactive:
            break;
        case Private::Accepting:
            d->intermediate << Private::CatcherResult(description, image);
            break;
        case Private::Waiting:
            d->harvest(description, image);
            break;
        case Private::Quitting:
            break;
    }
}

int ThumbnailImageCatcher::enqueue()
{
    QList<LoadingDescription> descriptions = d->thread->lastDescriptions();

    QMutexLocker lock(&d->mutex);

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
    d->state = Private::Waiting;

    // first, handle results received between request and calling this method
    foreach(const Private::CatcherResult& result, d->intermediate)
    {
        d->harvest(result.description, result.image);
    }

    d->intermediate.clear();

    // Now wait for the rest to arrive. If already finished, state will be Quitting
    while (d->state == Private::Waiting)
    {
        d->condVar.wait(&d->mutex);
    }

    QList<QImage> result;

    foreach(const Private::CatcherResult& task, d->tasks)
    {
        result << task.image;
    }

    d->reset();

    return result;
}

}   // namespace Digikam
