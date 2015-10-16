/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to interface digiKam with kipi library.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well dot com>
 * Copyright (C) 2004-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kipiinterface.h"

// Qt includes

#include <QAbstractItemModel>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Libkipi includes

#include <KIPI/ImageCollection>
#include <KIPI/ImageInfoShared>
#include <KIPI/ImageCollectionShared>

// LibDRawDecoder includes

#include "drawdecoder.h"

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albumdb.h"
#include "applicationsettings.h"
#include "dcrawsettingswidget.h"
#include "imageinfo.h"
#include "metadatasettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "digikamview.h"
#include "loadingcacheinterface.h"
#include "previewloadthread.h"
#include "filereadwritelock.h"
#include "scancontroller.h"
#include "imageattributeswatch.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "kipiimageinfo.h"
#include "kipiimagecollection.h"
#include "progressmanager.h"
#include "dimg.h"

namespace Digikam
{

class KipiInterface::Private
{
public:

    Private()
    {
        tagModel        = 0;
        thumbLoadThread = 0;
        previewThread   = 0;
        albumManager    = 0;
    }

    AlbumManager*        albumManager;
    ThumbnailLoadThread* thumbLoadThread;
    PreviewLoadThread*   previewThread;
    QAbstractItemModel*  tagModel;
};

KipiInterface::KipiInterface(QObject* const parent, const QString& name)
    : KIPI::Interface(parent, name),
      d(new Private())
{
    d->previewThread   = new PreviewLoadThread(this);
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();
    d->albumManager    = AlbumManager::instance();

    connect(DigikamApp::instance()->view(), SIGNAL(signalSelectionChanged(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
    
    connect(d->previewThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription,DImg)));
}

KipiInterface::~KipiInterface()
{
    delete d;
}

KIPI::ImageCollection KipiInterface::currentAlbum()
{
    if (d->albumManager->currentAlbums().isEmpty())
    {
        return KIPI::ImageCollection(0);
    }

    Album* const currAlbum = d->albumManager->currentAlbums().first();

    if (currAlbum)
    {
        return KIPI::ImageCollection(new KipiImageCollection(KipiImageCollection::AllItems,
                                                             currAlbum,
                                                             hostSetting(QLatin1String("FileExtensions")).toString()));
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

KIPI::ImageCollection KipiInterface::currentSelection()
{
    if (d->albumManager->currentAlbums().isEmpty())
    {
            return KIPI::ImageCollection(0);
    }

    Album* const currAlbum = d->albumManager->currentAlbums().first();

    if (currAlbum)
    {
        return KIPI::ImageCollection(new KipiImageCollection(KipiImageCollection::SelectedItems,
                                                             currAlbum,
                                                             hostSetting(QLatin1String("FileExtensions")).toString()));
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

QList<KIPI::ImageCollection> KipiInterface::allAlbums()
{
    QList<KIPI::ImageCollection> result;
    QString fileFilter(hostSetting(QLatin1String("FileExtensions")).toString());

    const AlbumList palbumList = d->albumManager->allPAlbums();

    for (AlbumList::ConstIterator it = palbumList.constBegin();
         it != palbumList.constEnd(); ++it)
    {
        // don't add the root album
        if ((*it)->isRoot())
        {
            continue;
        }

        KipiImageCollection* const col = new KipiImageCollection(KipiImageCollection::AllItems, *it, fileFilter);
        result.append(KIPI::ImageCollection(col));
    }

    const AlbumList talbumList = d->albumManager->allTAlbums();

    for (AlbumList::ConstIterator it = talbumList.constBegin();
         it != talbumList.constEnd(); ++it)
    {
        // don't add the root album
        if ((*it)->isRoot())
        {
            continue;
        }

        KipiImageCollection* const col = new KipiImageCollection(KipiImageCollection::AllItems, *it, fileFilter);
        result.append(KIPI::ImageCollection(col));
    }

    return result;
}

KIPI::ImageInfo KipiInterface::info(const QUrl& url)
{
    return KIPI::ImageInfo(new KipiImageInfo(this, url));
}

void KipiInterface::refreshImages(const QList<QUrl>& urls)
{
    QList<QUrl> ulist = urls;

    // Hard Refresh
    QSet<QString>    dirs;
    QList<qlonglong> ids;

    foreach(const QUrl& url, urls)
    {
        ImageInfo info = ImageInfo::fromUrl(url);

        if (!info.isNull())
        {
            ids << info.id();
        }

        QString path = url.toLocalFile();
        ThumbnailLoadThread::deleteThumbnail(path);
        LoadingCacheInterface::fileChanged(path);
        ImageAttributesWatch::instance()->fileMetadataChanged(url);
        dirs << url.adjusted(QUrl::RemoveFilename).path();
    }

    ScanController::instance()->hintAtModificationOfItems(ids);

    foreach(const QString& dir, dirs)
    {
        ScanController::instance()->scheduleCollectionScan(dir);
    }
}

int KipiInterface::features() const
{
    return(  KIPI::CollectionsHaveComments
           | KIPI::CollectionsHaveCategory
           | KIPI::CollectionsHaveCreationDate
           | KIPI::ImagesHasComments
           | KIPI::ImagesHasTitlesWritable
           | KIPI::ImagesHasTime
           | KIPI::HostSupportsTags
           | KIPI::HostSupportsRating
           | KIPI::HostAcceptNewImages
           | KIPI::HostSupportsThumbnails
           | KIPI::HostSupportsProgressBar
           | KIPI::HostSupportsReadWriteLock
           | KIPI::HostSupportsPickLabel
           | KIPI::HostSupportsColorLabel
           | KIPI::HostSupportsPreviews
           | KIPI::HostSupportsRawProcessing
          );
}

bool KipiInterface::addImage(const QUrl& url, QString& errmsg)
{
    // Note : All copy/move operations are processed by the plugins.

    if (url.isValid() == false)
    {
        errmsg = i18n("Target URL %1 is not valid.", url.toLocalFile());
        return false;
    }

    PAlbum* const targetAlbum = d->albumManager->findPAlbum(url.adjusted(QUrl::RemoveFilename));

    if (!targetAlbum)
    {
        errmsg = i18n("Target album is not in the album library.");
        return false;
    }

    //d->albumManager->refreshItemHandler( url );

    return true;
}

void KipiInterface::delImage(const QUrl& url)
{
    QUrl rootURL(CollectionManager::instance()->albumRoot(url));

    if (!rootURL.isParentOf(url))
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "URL not in the album library";
    }

    // Is there a PAlbum for this URL

    PAlbum* const palbum = d->albumManager->findPAlbum(url.adjusted(QUrl::RemoveFilename));

    if (palbum)
    {
        // delete the item from the database
        DatabaseAccess().db()->deleteItem(palbum->id(), url.fileName());
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot find Parent album in the album library";
    }
}

void KipiInterface::slotSelectionChanged(int count)
{
    emit selectionChanged(count);
}

void KipiInterface::slotCurrentAlbumChanged(QList<Album*> albums)
{
    emit currentAlbumChanged(!(albums.isEmpty()));
}

QImage KipiInterface::preview(const QUrl& url, int minSize)
{
    return (PreviewLoadThread::loadFastButLargeSynchronously(url.toLocalFile(), minSize).copyQImage());
}

void KipiInterface::preview(const QUrl& url, int minSize, int /*TODO resizedTo*/)
{
    d->previewThread->loadFastButLarge(url.toLocalFile(), minSize);
}

void KipiInterface::thumbnail(const QUrl& url, int /*size*/)
{
    // NOTE: size is not used here. Cache use the max pixmap size to store thumbs.
    d->thumbLoadThread->find(ImageInfo::fromUrl(url).thumbnailIdentifier());
}

void KipiInterface::thumbnails(const QList<QUrl>& list, int size)
{
    for (QList<QUrl>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        thumbnail(*it, size);
    }
}

void KipiInterface::slotGotImagePreview(const LoadingDescription& desc, const DImg& image)
{
    QUrl url = QUrl::fromLocalFile(desc.filePath);

    if (desc.isThumbnail() || !url.isValid())
    {
        return;
    }

    emit gotPreview(url, image.copyQImage());
}

void KipiInterface::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    emit gotThumbnail(QUrl::fromLocalFile(desc.filePath), pix);
}

KIPI::ImageCollectionSelector* KipiInterface::imageCollectionSelector(QWidget* parent)
{
    return (new KipiImageCollectionSelector(this, parent));
}

KIPI::UploadWidget* KipiInterface::uploadWidget(QWidget* parent)
{
    return (new KipiUploadWidget(this, parent));
}

QAbstractItemModel* KipiInterface::getTagTree() const
{

    if (!d->tagModel)
    {
        QAbstractItemModel* const newTagModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, NULL);
        d->tagModel                           = newTagModel;
    }

    return d->tagModel;
}

QVariant KipiInterface::hostSetting(const QString& settingName)
{
    MetadataSettings* const mSettings    = MetadataSettings::instance();
    ApplicationSettings* const aSettings = ApplicationSettings::instance();

    if (!mSettings || !aSettings)
    {
        return QVariant();
    }

    MetadataSettingsContainer set = mSettings->settings();

    if (settingName == QLatin1String("WriteMetadataUpdateFiletimeStamp"))
    {
        return (set.updateFileTimeStamp);
    }
    else if (settingName == QLatin1String("WriteMetadataToRAW"))
    {
        return (set.writeRawFiles);
    }
    else if (settingName == QLatin1String("UseXMPSidecar4Reading"))
    {
        return (set.useXMPSidecar4Reading);
    }
    else if (settingName == QLatin1String("MetadataWritingMode"))
    {
        return (set.metadataWritingMode);
    }
    else if (settingName == QLatin1String("FileExtensions"))
    {
        // NOTE : do not save type mime settings into a local variable, as this
        // might change in the main app

        return QString((aSettings->getImageFileFilter()) + QLatin1Char(' ') +
                       (aSettings->getMovieFileFilter()) + QLatin1Char(' ') +
                       (aSettings->getAudioFileFilter()) + QLatin1Char(' ') +
                       (aSettings->getRawFileFilter()));
    }
    else if (settingName == QLatin1String("ImagesExtensions"))
    {
        return aSettings->getImageFileFilter();
    }
    else if (settingName == QLatin1String("RawExtensions"))
    {
        return aSettings->getRawFileFilter();
    }
    else if (settingName == QLatin1String("VideoExtensions"))
    {
        return aSettings->getMovieFileFilter();
    }
    else if (settingName == QLatin1String("AudioExtensions"))
    {
        return aSettings->getAudioFileFilter();
    }

    return QVariant();
}

QString KipiInterface::progressScheduled(const QString& title, bool canBeCanceled, bool hasThumb) const
{
    ProgressItem* const item = ProgressManager::createProgressItem(title, QString(), canBeCanceled, hasThumb);

    if (canBeCanceled)
    {
        connect(item, SIGNAL(progressItemCanceled(QString)),
                this, SIGNAL(progressCanceled(QString)));
    }

    return item->id();
}

void KipiInterface::progressValueChanged(const QString& id, float percent)
{
    ProgressItem* const item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setProgress(percent);
    }
}

void KipiInterface::progressStatusChanged(const QString& id, const QString& status)
{
    ProgressItem* const item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setStatus(status);
    }
}

void KipiInterface::progressThumbnailChanged(const QString& id, const QPixmap& thumb)
{
    ProgressItem* const item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setThumbnail(thumb);
    }
}

void KipiInterface::progressCompleted(const QString& id)
{
    ProgressItem* const item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setComplete();
    }
}

void KipiInterface::aboutToEdit(const QUrl& url, KIPI::EditHints hints)
{
    if (hints == KIPI::HintMetadataOnlyChange)
    {
        ImageInfo info = ImageInfo::fromUrl(url);
        ScanController::instance()->beginFileMetadataWrite(info);
    }
}

void KipiInterface::editingFinished(const QUrl& url, KIPI::EditHints hints)
{
    if ((hints & ~KIPI::HintEditAborted) == KIPI::HintMetadataOnlyChange)
    {
        ImageInfo info = ImageInfo::fromUrl(url);
        ScanController::instance()->finishFileMetadataWrite(info, !(hints & KIPI::HintEditAborted));
    }
}

// ---------------------------------------------------------------------------------------

class KipiInterfaceFileReadWriteLock : public KIPI::FileReadWriteLock
{
public:

    explicit KipiInterfaceFileReadWriteLock(const QString& filePath)
        : key(filePath)
    {
    }

    ~KipiInterfaceFileReadWriteLock()
    {
    }

public:

    void lockForRead()                { key.lockForRead();                   }
    void lockForWrite()               { key.lockForWrite();                  }
    bool tryLockForRead()             { return key.tryLockForRead();         }
    bool tryLockForRead(int timeout)  { return key.tryLockForRead(timeout);  }
    bool tryLockForWrite()            { return key.tryLockForWrite();        }
    bool tryLockForWrite(int timeout) { return key.tryLockForWrite(timeout); }
    void unlock()                     { key.unlock();                        }

public:

    FileReadWriteLockKey key;
};

KIPI::FileReadWriteLock* KipiInterface::createReadWriteLock(const QUrl& url) const
{
    return new KipiInterfaceFileReadWriteLock(url.toLocalFile());
}

// ---------------------------------------------------------------------------------------

class KipiInterfaceRawProcessor : public KIPI::RawProcessor
{
public:

    KipiInterfaceRawProcessor()
    {
    }

    ~KipiInterfaceRawProcessor()
    {
    }

    bool loadRawPreview(const QUrl& url, QImage& image)
    {
        return( decoder.loadRawPreview(image, url.toLocalFile()) );
    }

    bool decodeRawImage(const QUrl& url, QByteArray& imageData, int& width, int& height, int& rgbmax)
    {
        DRawDecoderSettings settings;
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
        DcrawSettingsWidget::readSettings(settings, group);
        return( decoder.decodeRAWImage(url.toLocalFile(), settings, imageData, width, height, rgbmax) );
    }

    void cancel()
    {
    }
    
    bool isRawFile(const QUrl& url)
    {
        QString   rawFilesExt(rawFiles());
        QFileInfo fileInfo(url.toLocalFile());

        return (rawFilesExt.toUpper().contains(fileInfo.suffix().toUpper()));
    }
    
    QString rawFiles()
    {
        return QLatin1String(decoder.rawFiles());
    }

private:

    RawEngine::DRawDecoder decoder;
};

KIPI::RawProcessor* KipiInterface::createRawProcessor() const
{
    return new KipiInterfaceRawProcessor;
}

}  // namespace Digikam
